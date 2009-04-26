#pragma once

#include <google/type_traits.h>
#include <google/dense_hash_set>
#include <google/dense_hash_map>
#include <google/sparsehash/densehashtable.h>

namespace HashTools {

#define HashFriend(Key,T) friend class HashMap<Key,T>

/// Defines an equality comparison unary method.
/// Generally intended for internal use only.
#define _EQUALS_UNARY_OP( Type ) bool operator()(const Type s1, const Type s2) const { return s1.Equals( s2 ); }

/// Defines a hash code unary method
/// Generally intended for internal use only.
#define _HASHCODE_UNARY_OP( Type ) hash_key_t operator()( const Type& val ) const { return val.GetHashCode(); }

/// <summary>
///   Defines an equality comparison method within an encapsulating struct, using the 'unary method' approach.
/// </summary>
/// <remarks>
///   <para>
///     This macro is a shortcut helper to implementing types usable as keys in <see cref="HashMap"/>s.
///     Normally you will want to use <see cref="DEFINE_HASH_API"/> instead as it defines both
///     the HashCode predicate and Compare predicate.
///   </para>
///   The code generated by this macro is equivalent to this:
///   <code>
///		// where 'Type' is the parameter used in the macro.
///		struct UnaryEquals
///		{
///			bool operator()(const Type s1, const Type s2) const
///			{
///				return s1.Equals( s2 );			// this operator must be implemented by the user.
///			}
///		};
///   </code>
///   Note:
///   In C++, the term 'unary method' refers to a method that is implemented as an overload of the
///   <c>operator ()</c>, such that the object instance itself acts as a method.
///   Note:
///   This methodology is similar to C# / .NET's <c>object.Equals()</c> method: The class member method
///   implementation of <c>Equals</c> should *not* throw exceptions -- it should instead return <c>false</c>
///   if either side of the comparison is not a matching type.  See <see cref="IHashable" /> for details.
///   Note:
///   The reason for this (perhaps seemingly) hogwash red tape is because you can define custom
///   equality behavior for individual hashmaps, which are independent of the type used.   The only
///   obvious scenario where such a feature is useful is in 
/// </remarks>
/// <seealso cref="DEFINE_HASHCODE_UNARY"/>
/// <seealso cref="DEFINE_HASH_API"/>
/// <seealso cref="IHashable"/>
/// <seealso cref="HashMap"/>
#define DEFINE_EQUALS_UNARY( Type ) struct UnaryEquals{ _EQUALS_UNARY_OP( Type ) }

/// <summary>
///   Defines a hash code predicate within an encapsulating struct; for use in hashable user datatypes
/// </summary>
/// <remarks>
///   <para>
///     This macro is a shortcut helper to implementing types usable as keys in <see cref="HashMap"/>s.
///     Normally you will want to use <see cref="DEFINE_HASH_API"/> instead as it defines both
///     the HashCode predicate and Compare predicate.
///   </para>
///   The code generated by this macro is equivalent to this:
///   <code>
///		// where 'Type' is the parameter used in the macro.
///		struct UnaryHashCode
///		{
///			hash_key_t operator()( const Type& val ) const
///			{
///				return val.GetHashCode();		// this member function must be implemented by the user.
///			}
///		};
///   </code>
/// </remarks>
/// <seealso cref="DEFINE_EQUALS_UNARY"/>
/// <seealso cref="DEFINE_HASH_API"/>
/// <seealso cref="IHashable"/>
/// <seealso cref="HashMap"/>
#define DEFINE_HASHCODE_UNARY( Type ) struct UnaryHashCode{ _HASHCODE_UNARY_OP( Type ) }

/// <summary>
///   Defines the API for hashcode and comparison unary methods; for use in hashable user datatypes
/// </summary>
/// <remarks>
///   This macro creates APIs that allow the class or struct to be used as a key in a <see cref="HashMap"/>.
///   It requires that the data type implement the following items:
///    * An equality test via an <c>operator==</c> overload.
///    * A public instance member method <c>GetHashCode.</c>
///   The code generated by this macro is equivalent to this:
///   <code>
///		// where 'Type' is the parameter used in the macro.
///		struct UnaryHashCode
///		{
///			hash_key_t operator()( const Type& val ) const
///			{
///				return val.GetHashCode();		// this member function must be implemented by the user.
///			}
///		};
/// 
///		struct UnaryEquals
///		{
///			bool operator()(const Type s1, const Type s2) const
///			{
///				return s1.Equals( s2 );			// this operator must be implemented by the user.
///			}
///		};
///   </code>
///   Note:
///   In C++, the term 'unary method' refers to a method that is implemented as an overload of the
///   <c>operator ()</c>, such that the object instance itself acts as a method.
///   Note:
///   For class types you can use the <see cref="IHashable"/> interface, which also allows you to group
///   multiple types of objects into a single complex HashMap.
///   Note:
///   Generally speaking, you do not use the <c>IHashable</c> interface on simple C-style structs, since it
///   would incur the overhead of a vtbl and could potentially break code that assumes the structs to have
///   1-to-1 data-to-declaration coorlations.
///   Note:
///   Internally, using this macro is functionally equivalent to using both <see cref="DEFINE_HASHCODE_CLASS"/>
///   and <see cref="DEFINE_EQUALS_CLASS"/>.
/// </remarks>
/// <seealso cref="IHashable"/>
/// <seealso cref="DEFINE_HASHCODE_CLASS"/>
/// <seealso cref="DEFINE_COMPARE_CLASS"/>
/// <seealso cref="DEFINE_HASH_API"/>
/// <seealso cref="HashMap"/>
#define DEFINE_HASH_API( Type ) DEFINE_HASHCODE_UNARY( Type ); DEFINE_EQUALS_UNARY( Type );

/// <summary>
///   A helper macro for creating custom types that can be used as <see cref="HashMap" /> keys.
/// </summary>
/// <remarks>
///   Use of this macro is only needed if the hashable type in question is a struct that is a private
///   local to the namespace of a containing class.
/// </remarks>	
#define PRIVATE_HASHMAP( Key, T ) \
	typedef SpecializedHashMap<Key, T> Key##HashMap; \
	friend Key##HashMap;

/// <summary>
///   Type that represents a hashcode; returned by all hash functions.
/// </summary>
/// <remarks>
///   In theory this could be changed to a 64 bit value in the future, although many of the hash algorithms
///   would have to be changed to take advantage of the larger data type.
/// </remarks>
typedef u32 hash_key_t;

hash_key_t Hash(const char* data, int len);

struct CommonHashClass;
extern const CommonHashClass GetCommonHash;

/// <summary>
///   A unary-style set of methods for getting the hash code of C++ fundamental types.
/// </summary>
/// <remarks>
///   This class is used to pass hash functions into the <see cref="HashMap"/> class and
///   it's siblings.  It houses methods for most of the fundamental types of C++ and the STL,
///   such as all int and float types, and also <c>std::string</c>.  All functions can be
///	  accessed via the () overload on an instance of the class, such as:
///   <code>
///		const CommonHashClass GetHash;
///		int v = 27;
///		std::string s = "Joe's World!";
///		hash_key_t hashV = GetHash( v );
///		hash_key_t hashS = GetHash( s );
///	  </code>
///   Note:
///   In C++, the term 'unary method' refers to a method that is implemented as an overload of the
///   <c>operator ()</c>, such that the object instance itself acts as a method.
/// </remarks>
/// <seealso cref="GetCommonHash"/>
struct CommonHashClass
{
public:
	hash_key_t operator()(const std::string& src) const
	{
		return Hash( src.data(), src.length() );
	}

	hash_key_t operator()( const std::wstring& src ) const
	{
		return Hash( (const char *)src.data(), src.length() * sizeof( wchar_t ) );
	}
	
	// Returns a hashcode for a character.
	// This has function has been optimized to return an even distribution
	// across the range of an int value.  In theory that should be more rewarding
	// to hastable performance than a straight up char lookup.
	hash_key_t operator()( const char c1 ) const
	{
		// Most chars contain values between 0 and 128, so let's mix it up a bit:
		int cs = (int)( c1 + (char)64 );
		return ( cs + ( cs<<8 ) + ( cs << 16 ) + (cs << 24 ) );
	}
	
	hash_key_t operator()( const wchar_t wc1 ) const
	{
		// Most unicode values are between 0 and 128, with 0-1024
		// making up the bulk of the rest.  Everything else is spatially used.
		/*int wcs = (int) ( wc1 + 0x2000 );
		return wcs ^ ( wcs + 0x19000 );*/
		
		// or maybe I'll just feed it into the int hash:
		return GetCommonHash( (u32)wc1 );
	}

	/// <summary>
	///   Gets the hash code for a 32 bit integer.
	/// </summary>
	/// <remarks>
	///   This method performs a very fast algorithm optimized for typical integral
	///   dispersion patterns (which tend to favor a bit heavy on the lower-range of values while
	///   leaving the extremes un-used).
	///   Note:
	///   Implementation is based on an article found here: http://www.concentric.net/~Ttwang/tech/inthash.htm
	/// </remarks>
	hash_key_t operator()( const u32 val ) const
	{
		u32 key = val;
		key = ~key + (key << 15);
		key = key ^ (key >> 12);
		key = key + (key << 2);
		key = key ^ (key >> 4);
		key = key * 2057;
		key = key ^ (key >> 16);
		return key;
	}

	/// <summary>
	///   Gets the hash code for a 32 bit integer.
	/// </summary>
	/// <remarks>
	///   This method performs a very fast algorithm optimized for typical integral
	///   dispersion patterns (which tend to favor a bit heavy on the lower-range of values while
	///   leaving the extremes un-used).
	///   Note:
	///   Implementation is based on an article found here: http://www.concentric.net/~Ttwang/tech/inthash.htm
	/// </remarks>
	hash_key_t operator()( const s32 val ) const
	{
		return GetCommonHash((u32)val);
	}

	/// <summary>
	///   Gets the hash code for a 64 bit integer.
	/// </summary>
	/// <remarks>
	///   This method performs a very fast algorithm optimized for typical integral
	///   dispersion patterns (which tend to favor a bit heavy on the lower-range of values while
	///   leaving the extremes un-used).
	///   Note:
	///   Implementation is based on an article found here: http://www.concentric.net/~Ttwang/tech/inthash.htm
	/// </remarks>
	hash_key_t operator()( const u64 val ) const
	{
		u64 key = val;
		key = (~key) + (key << 18);
		key = key ^ (key >> 31);
		key = key * 21;  // key = (key + (key << 2)) + (key << 4);
		key = key ^ (key >> 11);
		key = key + (key << 6);
		key = key ^ (key >> 22);
		return (u32) key;
	}

	/// <summary>
	///   Gets the hash code for a 64 bit integer.
	/// </summary>
	/// <remarks>
	///   This method performs a very fast algorithm optimized for typical integral
	///   dispersion patterns (which tend to favor a bit heavy on the lower-range of values while
	///   leaving the extremes un-used).
	///   Note:
	///   Implementation is based on an article found here: http://www.concentric.net/~Ttwang/tech/inthash.htm
	/// </remarks>
	hash_key_t operator()( const s64 val ) const
	{
		return GetCommonHash((u64)val);
	}
	
	hash_key_t operator()( const float val ) const
	{
		// floats do a fine enough job of being scattered about
		// the universe:
		return *((hash_key_t *)&val);
	}

	hash_key_t operator()( const double val ) const
	{
		// doubles have to be compressed into a 32 bit value:
		return GetCommonHash( *((u64*)&val) );
	}

	/// <summary>
	///   Calculates the hash of a pointer.
	/// </summary>
	/// <remarks>
	///   This method has been optimized to give typical 32 bit pointers a reasonably
	///   wide spread across the integer spectrum.
	///   Note:
	///   This method is optimized for 32 bit pointers only.  64 bit pointer support
	///   has not been implemented, and thus on 64 bit platforms performance could be poor or,
	///   worse yet, results may not have a high degree of uniqueness.
	/// </remarks>
	hash_key_t operator()( const void* addr ) const
	{
		hash_key_t key = (hash_key_t) addr;
		return (hash_key_t)((key >> 3) * 2654435761ul);
	}
};

/// <summary>
///   This class contains comparison methods for most fundamental types; and is used by the CommonHashMap class.
/// </summary>
/// <remarks>
///   The predicates of this class do standard equality comparisons between fundamental C/STL types such as
///   <c>int, float</c>, and <c>std::string.</c>  Usefulness of this class outside the <see cref="CommonHashMap"/>
///   class is limited.
/// </remarks>
/// <seealso cref="CommonHashMap">
struct CommonComparisonClass
{
	bool operator()(const char* s1, const char* s2) const
	{
		return (s1 == s2) || (s1 && s2 && strcmp(s1, s2) == 0);
	}
};

/// <summary>
///   An interface for classes that implement hashmap functionality.
/// </summary>
/// <remarks>
///   This class provides interface methods for getting th hashcode of a class and checking for object
///   equality.  It's general intent is for use in situations where you have to store *non-similar objects*
///   in a single unified hash map.  As all object instances derive from this type, it allows the equality
///   comparison to use typeid or dynamic casting to check for type similarity, and then use more detailed
///   equality checks for similar types.
/// </remarks>
class IHashable
{
public:
	/// Obligatory Virtual destructor mess!
	virtual ~IHashable() {};

	/// <summary>
	///   Your basic no-thrills equality comparison; using a pointer comparison by default.
	/// </summary>
	/// <remarks>
	///	  This method uses a pointer comparison by default, which is the only way to really compare objects
	///   of unrelated types or of derrived types.  When implementing this method, you may want to use typeid comparisons
	///   if you want derived types to register as being non-equal, or <c>dynamic_cast</c> for a more robust
	///   base-class comparison (illustrated in the example below).
	///   Note:
	///   It's recommended important to always do a pointer comparison as the first step of any object equality check.
	///   It is fast and easy, and 100% reliable.
	/// </remarks>
	/// <example>
	///   Performing non-pointer comparisons:
	///   <code>
	///		class Hasher : IHashable
	///		{
	///			int  someValue;
	/// 	
	///			virtual bool Equals( const IHashable& right ) const
	/// 		{
	///				// Use pointer comparison first since it's fast and accurate:
	/// 			if( &right == this ) return true;
	///
	///				Hasher* them = dynamic_cast&lt;Hasher*&gt;( right );
	///				if( them == NULL ) return false;
	/// 			return someValue == them->SomeValue;
	/// 		}
	///		}
	///   </code>
	/// </example>
	virtual bool Equals( const IHashable& right ) const
	{
		return ( &right == this );		// pointer comparison.
	}

	/// <summary>
	///   Returns a hash value for this object; by default the hash of its pointer address.
	/// </summary>
	/// <remarks>
	/// </remarks>
	/// <seealso cref="HashMap"/>
	virtual hash_key_t GetHashCode() const
	{
		return GetCommonHash( this );
	}
};

template< typename Key >
class HashSet : public google::dense_hash_set< Key, CommonHashClass >
{
public:
	/// <summary>
	///   Constructor.
	/// </summary>
	/// <remarks>
	///   Both the <c>emptyKey</c>a nd c>deletedKey</c> parameters must be unique values that
	///   are *not* used as actual values in the set.
	/// </remarks>
	HashSet( Key emptyKey, Key deletedKey, int initialCapacity=33 ) :
		google::dense_hash_set<Key, CommonHashClass>( initialCapacity )
	{
		set_empty_key( emptyKey );
		set_deleted_key( deletedKey );
	}
};

/// <summary>
///   Defines a hashed collection of objects and provides methods for adding, removing, and reading items.
/// </summary>
/// <remarks>
///   <para>This class is for hashing out a set data using objects as keys.  Objects should derive from the
///     <see cref="IHashable"/> type, and in either case *must* implement the UnaryHashCode and UnaryEquals
///     unary classes.</para>
///   <para>*Details On Implementing Key Types*</para>
///   <para>
///     Custom hash keying uses what I consider a somewhat contrived method of implementing the Key type;
///     involving a handful of macros in the best case, and a great deal of syntaxical red tape in
///     the worst case.  Most cases should fall within the realm of the macros, which make life a lot easier,
///     so that's the only implementation I will cover in detail here (see below for example).
///   </para>
///   Note:
///   For most hashs based on common or fundamental types or types that can be adequately compared using
///   the default equality operator ==, such as <c>int</c> or structs that have no padding alignment concerns,
///   use <see cref="HashMap" /> instead.  For string-based hashs, use <see cref="Dictionary" /> or <see cref="UnicodeDictionary" />.
/// </remarks>
/// <example>
///   This is an example of making a hashable type out of a struct.  This is useful in situations where
///   inheriting the <see cref="IHashable"/> type would cause unnecessary overhead and/or broken C/C++
///   compatability.
///   <code>
/// 	struct Point
/// 	{
/// 		int x, y;
/// 		
/// 		// Empty constructor is necessary for HashMap.
/// 		// This can either be initialized to zero, or uninitialized as here:
/// 		Point() {}
/// 		
/// 		// Copy Constructor is just always necessary.
/// 		Point( const Point& src ) : first( src.first ), second( src.second ) {}
/// 
/// 		// Standard content constructor (Not needed by HashMap)
/// 		Point( int xpos, int ypos ) : x( xpos ), y( ypos ) {}
/// 			
/// 		/****  Begin Hashmap Interface Implementation ****/
/// 
/// 		// HashMap Requires both GetEmptyKey() and GetDeleteKey() instance member
/// 		// methods to be defined.  These act as defaults.  The actual values used
/// 		// can be overridden on an individual HashMap basis via the HashMap constructor.
/// 
/// 		static Point GetEmptyKey() { return Point( -0xffffff, 0xffffff ); }
/// 		static Point GetDeletedKey() { return Kerning( -0xffffee, 0xffffee ); }
/// 
/// 		// HashMap Requires an Equality Overload.
/// 		// The inequality overload is not required but is probably a good idea since
/// 		// orphaned equality (without sibling inequality) operator overloads are ugly code.
/// 
/// 		bool Equals( const Point& right ) const
/// 		{
/// 			return ( x == right.x ) && ( y == right.y );
/// 		}
/// 
/// 		hash_key_t GetHashCode() const
/// 		{
/// 			// This is a decent "universal" hash method for when you have multiple int types:
/// 			return GetCommonHash( x ) ^ GetCommonHash( y );
/// 		}
/// 
/// 		// Use a macro to expose the hash API to the HashMap templates.
/// 		// This macro creates MakeHashCode and Compare structs, which use the ()
/// 		// operator to create "unary methods" for the GetHashCode and == operator above.
/// 		// Feeling dizzy yet?  Don't worry.  Just follow this template.  It works!
/// 		
/// 		DEFINE_HASH_API( Point );
/// 		
/// 		/**** End HashMap Interface Implementation ****/
/// 	};
///   </code>
/// </example>
template< class Key, class T >
class SpecializedHashMap : public google::dense_hash_map<Key, T, typename Key::UnaryHashCode, typename Key::UnaryEquals>
{
public:
	virtual ~SpecializedHashMap() {}
	SpecializedHashMap( int initialCapacity=33, Key emptyKey=Key::GetEmptyKey(), Key deletedKey=Key::GetDeletedKey() ) :
		google::dense_hash_map<Key, T, typename Key::UnaryHashCode, typename Key::UnaryEquals>( initialCapacity )
	{
		set_empty_key( emptyKey );
		set_deleted_key( deletedKey );
	}

	/// <summary>
	///   Tries to get a value from this hashmap; or does nothing if the Key does not exist.
	/// </summary>
	/// <remarks>
	///   If found, the value associated with the requested key is copied into the <c>outval</c>
	///   parameter.  This is a more favorable alternative to the indexer operator since the
	///   indexer implementation can and will create new entries for every request that 
	/// </remarks>
	void TryGetValue( const Key& key, T& outval ) const
	{
		const_iterator iter = find( key );
		if( iter != end() )
			outval = iter->second;
	}
	
	const T& GetValue( Key key ) const
	{
		return (this->find( key ))->second;
	}
};

/// <summary>
///   This class implements a hashmap that uses fundamental types such as <c>int</c> or <c>std::string</c>
///   as keys.
/// </summary>
/// <remarks>
///   This class is provided so that you don't have to jump through hoops in order to use fundamental types as
///   hash keys.  The <see cref="HashMap" /> class isn't suited to the task since it requires the key type to
///   include a set of unary methods.  Obviously predicates cannot be added to fundamentals after the fact. :)
///   Note:
///   Do not use <c>char *</c> or <c>wchar_t *</c> as key types.  Use <c>std::string</c> and <c>std::wstring</c> instead,
///   as performance of those types will generally be superior.  For that matter, don't use this class at all!
///   Use the string-specialized classes <see cref="Dictionary" /> and <see cref="UnicodeDictionary" />.
/// </remarks>
template< class Key, class T >
class HashMap : public google::dense_hash_map<Key, T, CommonHashClass>
{
public:
	virtual ~HashMap() {}

	/// <summary>
	///   Constructor.
	/// </summary>
	/// <remarks>
	///   Both the <c>emptyKey</c>a nd c>deletedKey</c> parameters must be unique values that
	///   are *not* used as actual values in the set.
	/// </remarks>
	HashMap( Key emptyKey, Key deletedKey, int initialCapacity=33 ) :
		google::dense_hash_map<Key, T, CommonHashClass>( initialCapacity )
	{
		set_empty_key( emptyKey );
		set_deleted_key( deletedKey );
	}

	/// <summary>
	///   Tries to get a value from this hashmap; or does nothing if the Key does not exist.
	/// </summary>
	/// <remarks>
	///   If found, the value associated with the requested key is copied into the <c>outval</c>
	///   parameter.  This is a more favorable alternative to the indexer operator since the
	///   indexer implementation can and will create new entries for every request that 
	/// </remarks>
	void TryGetValue( const Key& key, T& outval ) const
	{
		const_iterator iter = find( key );
		if( iter != end() )
			outval = iter->second;
	}
	
	const T& GetValue( Key key ) const
	{
		return (this->find( key ))->second;
	}
};

/// <summary>
///   A shortcut class for easy implementation of string-based hash maps.
/// </summary>
/// <remarks>
///   Note:
///   This class does not support Unicode character sets natively.  To use Unicode strings as keys,
///   use <see cref="UnicodeDictionary"/> instead.
/// </remarks>
template< class T >
class Dictionary : public HashMap<std::string, T>
{
public:
	virtual ~Dictionary() {}

	Dictionary( int initialCapacity=33, std::string emptyKey = "@@-EMPTY-@@", std::string deletedKey = "@@-DELETED-@@" ) :
		HashMap( emptyKey, deletedKey, initialCapacity)
	{
	}
private:
	Dictionary( const Dictionary& src ) {}
};

/// <summary>
///   A shortcut class for easy implementation of string-based hash maps.
/// </summary>
/// <remarks>
///   Note:
///   This class does incur some amount of additional overhead over <see cref="Dictionary"/>, as it
///   requires twice as much memory and much hash twice as much data.
///   If you're only using the hash for friendly named array access (via string constants)
///   then you should probably just stick to using the regular dictionary.
/// </remarks>
template< class T >
class UnicodeDictionary : public HashMap<std::wstring, T>
{
public:
	virtual ~UnicodeDictionary() {}

	UnicodeDictionary( int initialCapacity=33, std::wstring emptyKey = "@@-EMPTY-@@", std::wstring deletedKey = "@@-DELETED-@@" ) :
		HashMap( emptykey, deletedkey, initialCapacity)
	{
	}		

private:
	UnicodeDictionary( const UnicodeDictionary& src ) {}
};

}