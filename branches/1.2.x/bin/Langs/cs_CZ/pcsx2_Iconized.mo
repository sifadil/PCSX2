��    J      l  e   �      P  �   Q  9   �  Y     R   o  b   �  b   %  w   �  o    	  �   p	  �   >
  ~   �
  �   Z  `   !  Q   �  �  �  �   |  �     /   �  M     &  d  ]   �  �   �  G   �  H   �  d   &  �   �  b     �   �  �   P  5   �  6     �   E  �   =  �   �  �   �  �   Y  r  �  |   n  �   �  �  �  N  g  �   �  �   �   �   �!  �   8"  �   �"  z  �#  J   .%  h   y%  �   �%  p   w&  G  �&  �   0(  �   �(  �   Q)  �   �)  �   �*  �   E+  �   3,  �   �,  �   �-  �   U.  *  �.  �   0  �   1  \   �1  �   T2  r   �2  �   n3  �   �3  �   �4  �   �5  �  D6  B  �7  h   :  =   {:  m   �:  \   ';  n   �;  g   �;  �   [<  t   �<  �   X=  �   =>  �   �>  �   �?  _   k@  `   �@  �  ,A  �   C  �   �C  9   �D  ]   �D  �   >E  d   )F  �   �F  P   IG  P   �G  l   �G  �   XH  s   �H  �   gI  �   6J  F   �J  F   &K  �   mK  �   cL  �   �L  �   �M  �   �N  �  BO  �   Q  ;  �Q  �  �R  �  tT  M  V  �   RW  �   AX  �   Y    �Y  �  �Z  ?   s\  P   �\  �   ]  {   �]  �  ^  �   �_  �   :`  �   a  �   �a  �   �b    Nc  �   ad  �   e  �   �e  �   �f  C  ?g  /  �h  �   �i  Y   �j  �   k  �   �k  �   Ol  6  �l  �   n  �   �n  �  �o            2      7   (   :          +       -       =                  3   0       .         #   J                       ?         5   ,       !   4          6      
   E   $       <                       D   C       /          F       '   %   *   "                     &   1              H   B          G   9         A      ;   )       I         8   >               	   @    'Ignore' to continue waiting for the thread to respond.
'Cancel' to attempt to cancel the thread.
'Terminate' to quit PCSX2 immediately.
 0 - Disables VU Cycle Stealing.  Most compatible setting! 1 - Default cyclerate. This closely matches the actual speed of a real PS2 EmotionEngine. 1 - Mild VU Cycle Stealing.  Lower compatibility, but some speedup for most games. 2 - Moderate VU Cycle Stealing.  Even lower compatibility, but significant speedups in some games. 2 - Reduces the EE's cyclerate by about 33%.  Mild speedup for most games with high compatibility. 3 - Maximum VU Cycle Stealing.  Usefulness is limited, as this will cause flickering visuals or slowdown in most games. 3 - Reduces the EE's cyclerate by about 50%.  Moderate speedup, but *will* cause stuttering audio on many FMVs. All plugins must have valid selections for %s to run.  If you are unable to make a valid selection due to missing plugins or an incomplete install of %s, then press Cancel to close the Configuration panel. Avoids memory card corruption by forcing games to re-index card contents after loading from savestates.  May not be compatible with all games (Guitar Hero). Check HDLoader compatibility lists for known games that have issues with this. (Often marked as needing 'mode 1' or 'slow DVD' Check this to force the mouse cursor invisible inside the GS window; useful if using the mouse as a primary control device for gaming.  By default the mouse auto-hides after 2 seconds of inactivity. Completely closes the often large and bulky GS window when pressing ESC or pausing the emulator. Enable this if you think MTGS thread sync is causing crashes or graphical errors. Enables Vsync when the framerate is exactly at full speed. Should it fall below that, Vsync gets disabled to avoid further performance penalties. Note: This currently only works well with GSdx as GS plugin and with it configured to use DX10/11 hardware rendering. Any other plugin or rendering mode will either ignore it or produce a black frame that blinks whenever the mode switches. It also requires Vsync to be enabled. Enables automatic mode switch to fullscreen when starting or resuming emulation. You can still toggle fullscreen display at any time using alt-enter. Existing %s settings have been found in the configured settings folder.  Would you like to import these settings or overwrite them with %s default values?

(or press Cancel to select a different settings folder) Failed: Destination memory card '%s' is in use. Failed: Duplicate is only allowed to an empty PS2-Port or to the file system. Gamefixes can work around wrong emulation in some titles. 
They may also cause compatibility or performance issues. 

It's better to enable 'Automatic game fixes' at the main menu instead, and leave this page empty. 
('Automatic' means: selectively use specific tested fixes for specific games) Known to affect following games:
 * Bleach Blade Battler
 * Growlanser II and III
 * Wizardry Known to affect following games:
 * Digital Devil Saga (Fixes FMV and crashes)
 * SSX (Fixes bad graphics and crashes)
 * Resident Evil: Dead Aim (Causes garbled textures) Known to affect following games:
 * Mana Khemia 1 (Going "off campus")
 Known to affect following games:
 * Test Drive Unlimited
 * Transformers NTFS compression can be changed manually at any time by using file properties from Windows Explorer. NTFS compression is built-in, fast, and completely reliable; and typically compresses memory cards very well (this option is highly recommended). Note that when Framelimiting is disabled, Turbo and SlowMotion modes will not be available either. Note: Recompilers are not necessary for PCSX2 to run, however they typically improve emulation speed substantially. You may have to manually re-enable the recompilers listed above, if you resolve the errors. Notice: Due to PS2 hardware design, precise frame skipping is impossible. Enabling it will cause severe graphical errors in some games. Notice: Most games are fine with the default options. Notice: Most games are fine with the default options.  Out of Memory (sorta): The SuperVU recompiler was unable to reserve the specific memory ranges required, and will not be available for use.  This is not a critical error, since the sVU rec is obsolete, and you should use microVU instead anyway. :) PCSX2 is unable to allocate memory needed for the PS2 virtual machine. Close out some memory hogging background tasks and try again. PCSX2 requires a *legal* copy of the PS2 BIOS in order to run games.
You cannot use a copy obtained from a friend or the Internet.
You must dump the BIOS from your *own* Playstation 2 console. PCSX2 requires a PS2 BIOS in order to run.  For legal reasons, you *must* obtain a BIOS from an actual PS2 unit that you own (borrowing doesn't count).  Please consult the FAQs and Guides for further instructions. Playstation game discs are not supported by PCSX2.  If you want to emulate PSX games then you'll have to download a PSX-specific emulator, such as ePSXe or PCSX. Please ensure that these folders are created and that your user account is granted write permissions to them -- or re-run PCSX2 with elevated (administrator) rights, which should grant PCSX2 the ability to create the necessary folders itself.  If you do not have elevated rights on this computer, then you will need to switch to User Documents mode (click button below). Please select a valid BIOS.  If you are unable to make a valid selection then press Cancel to close the Configuration panel. Please select your preferred default location for PCSX2 user-level documents below (includes memory cards, screenshots, settings, and savestates).  These folder locations can be overridden at any time using the Plugin/BIOS Selector panel. Primarily targetting the EE idle loop at address 0x81FC0 in the kernel, this hack attempts to detect loops whose bodies are guaranteed to result in the same machine state for every iteration until a scheduled event triggers emulation of another unit.  After a single iteration of such loops, we advance to the time of the next event or the end of the processor's timeslice, whichever comes first. Removes any benchmark noise caused by the MTGS thread or GPU overhead.  This option is best used in conjunction with savestates: save a state at an ideal scene, enable this option, and re-load the savestate.

Warning: This option can be enabled on-the-fly but typically cannot be disabled on-the-fly (video will typically be garbage). Runs VU1 on its own thread (microVU1-only). Generally a speedup on CPUs with 3 or more cores. This is safe for most games, but a few games are incompatible and may hang. In the case of GS limited games, it may be a slowdown (especially on dual core CPUs). Setting higher values on this slider effectively reduces the clock speed of the EmotionEngine's R5900 core cpu, and typically brings big speedups to games that fail to utilize the full potential of the real PS2 hardware. Speedhacks usually improve emulation speed, but can cause glitches, broken audio, and false FPS readings.  When having emulation problems, disable this panel first. The PS2-slot %d has been automatically disabled.  You can correct the problem
and re-enable it at any time using Config:Memory cards from the main menu. The Presets apply speed hacks, some recompiler options and some game fixes known to boost speed.
Known important game fixes will be applied automatically.

--> Uncheck to modify settings manually (with current preset as base) The Presets apply speed hacks, some recompiler options and some game fixes known to boost speed.
Known important game fixes will be applied automatically.

Presets info:
1 -     The most accurate emulation but also the slowest.
3 --> Tries to balance speed with compatibility.
4 -     Some more aggressive hacks.
6 -     Too many hacks which will probably slow down most games.
 The specified path/directory does not exist.  Would you like to create it? The thread '%s' is not responding.  It could be deadlocked, or it might just be running *really* slowly. There is not enough virtual memory available, or necessary virtual memory mappings have already been reserved by other processes, services, or DLLs. This action will reset the existing PS2 virtual machine state; all current progress will be lost.  Are you sure? This command clears %s settings and allows you to re-run the First-Time Wizard.  You will need to manually restart %s after this operation.

WARNING!!  Click OK to delete *ALL* settings for %s and force-close the app, losing any current emulation progress.  Are you absolutely sure?

(note: settings for plugins are unaffected) This folder is where PCSX2 records savestates; which are recorded either by using menus/toolbars, or by pressing F1/F3 (save/load). This folder is where PCSX2 saves its logfiles and diagnostic dumps.  Most plugins will also adhere to this folder, however some older plugins may ignore it. This folder is where PCSX2 saves screenshots.  Actual screenshot image format and style may vary depending on the GS plugin being used. This hack works best for games that use the INTC Status register to wait for vsyncs, which includes primarily non-3D RPG titles. Games that do not use this method of vsync will see little or no speedup from this hack. This is the folder where PCSX2 saves your settings, including settings generated by most plugins (some older plugins may not respect this value). This recompiler was unable to reserve contiguous memory required for internal caches.  This error can be caused by low virtual memory resources, such as a small or disabled swapfile, or by another program that is hogging a lot of memory. This slider controls the amount of cycles the VU unit steals from the EmotionEngine.  Higher values increase the number of cycles stolen from the EE for each VU microprogram the game runs. This wizard will help guide you through configuring plugins, memory cards, and BIOS.  It is recommended if this is your first time installing %s that you view the readme and configuration guide. Updates Status Flags only on blocks which will read them, instead of all the time. This is safe most of the time, and Super VU does something similar by default. Vsync eliminates screen tearing but typically has a big performance hit. It usually only applies to fullscreen mode, and may not work with all GS plugins. Warning!  Changing plugins requires a complete shutdown and reset of the PS2 virtual machine. PCSX2 will attempt to save and restore the state, but if the newly selected plugins are incompatible the recovery may fail, and current progress will be lost.

Are you sure you want to apply settings now? Warning!  You are running PCSX2 with command line options that override your configured plugin and/or folder settings.  These command line options will not be reflected in the settings dialog, and will be disabled when you apply settings changes here. Warning!  You are running PCSX2 with command line options that override your configured settings.  These command line options will not be reflected in the Settings dialog, and will be disabled if you apply any changes here. Warning: Some of the configured PS2 recompilers failed to initialize and have been disabled: Warning: Your computer does not support SSE2, which is required by many PCSX2 recompilers and plugins. Your options will be limited and emulation will be *very* slow. When checked this folder will automatically reflect the default associated with PCSX2's current usermode setting.  You are about to delete the formatted memory card '%s'. All data on this card will be lost!  Are you absolutely and quite positively sure? You can change the preferred default location for PCSX2 user-level documents here (includes memory cards, screenshots, settings, and savestates).  This option only affects Standard Paths which are set to use the installation default value. You may optionally specify a location for your PCSX2 settings here.  If the location contains existing PCSX2 settings, you will be given the option to import or overwrite them. Your system is too low on virtual resources for PCSX2 to run. This can be caused by having a small or disabled swapfile, or by other programs that are hogging resources. Zoom = 100: Fit the entire image to the window without any cropping.
Above/Below 100: Zoom In/Out
0: Automatic-Zoom-In untill the black-bars are gone (Aspect ratio is kept, some of the image goes out of screen).
  NOTE: Some games draw their own black-bars, which will not be removed with '0'.

Keyboard: CTRL + NUMPAD-PLUS: Zoom-In, CTRL + NUMPAD-MINUS: Zoom-Out, CTRL + NUMPAD-*: Toggle 100/0 Project-Id-Version: PCSX2 0.9.9
Report-Msgid-Bugs-To: http://code.google.com/p/pcsx2/
POT-Creation-Date: 2014-02-01 18:19+0100
PO-Revision-Date: 2012-09-09 20:43+0100
Last-Translator: Zbyněk Schwarz <zbynek.schwarz@gmail.com>
Language-Team: Zbyněk Schwarz
Language: cs
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
Plural-Forms: nplurals=3; plural=(n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2;
X-Generator: Virtaal 0.7.1-rc1
X-Poedit-KeywordsList: pxE
X-Poedit-SourceCharset: utf-8
X-Poedit-SearchPath-0: pcsx2
X-Poedit-SearchPath-1: common
 'Ignorovat' pro pokračování čekání na odpověď vlákna.
'Zrušit' pro pokus o zrušení vlákna.
 Zakáže krádež cyklů VJ. Nejkompatibilnější nastavení Výchozí množství cyklů. Toto se blíže shoduje se skutečnou rychlostí opravdového EmotionEngine PS2. Mírná krádež cyklů VJ. Nižší kompatibilita, ale jisté zrychlení ve většině her. Průměrná krádež cyklů VJ. Ještě nižší kompatibilita, ale výrazné zrychlení v některých hrách. Sníží množství cyklů EE asi o 33%. Mírné zrychlení ve většině her s vysokou kompatibilitou. Maximální krádež cyklů VJ. Užitečnost je omezená protože toto způsobuje blikání grafiky nebo zpomalení ve většině her.  Sníží množství cyklů EE asi o 50%. Průměrné zrychlení, ale *způsobí* zadrhování zvuku ve spoustě FMV. Všechny zásuvné moduly musí mít platný výběr pro %s ke spuštění. Pokud nemůžete provést výběr kvůli chybějícímu modulu nebo nedokončené instalaci %s, pak stiskněte Zrušit pro uzavření panelu Nastavení. Zabraňuje poškození paměťové karty tím, že donutí hry reindexovat obsah karty po načtení uloženého stavu. Nemusí být kompatibilní se všemi hrami (Guitar Hero). Zkontrolujte seznam kompatibility HDLoadera pro hry, známé, že s tímto mají problémy. (často označené jako vyžadující 'mode 1' nebo 'slow DVD' Zašrktněte toto pro vynucení zneviditelnění kurzoru myši uvnitř okna GS; užitečné, jestli myš používáte jako hlavní kontrolní zařízení pro hraní. Standardně je myš schována po 2 vteřinách nečinnosti. Úplně zavře často velké a rozměrné okno GS při stisku ESC nebo pozastavení emulátoru. Zapněte toto, pokud si myslíte, že synch vlákna VVGS způsobuje pády a grafické problémy. Povolí Vsynch když snímkovací frekvence je přesně na plné rychlosti. Pokud spadne pod tuto hodnotu, Vsynch je zakázána k zabránění dalších penalizací výkonu. Poznámka: Toto nyní správně funguje pouze s GSdx jako zásuvný modul GS a nastaveným na použití hardwarového vykreslování DX10/11. Jakýkoli jiný modul nebo režim vykreslování toto bude ignorovat, nebo vytvoří černý snímek, který blikne, kdykoliv je režim přepnut. Také vyžaduje povolenou Vsynch. Povolí automatické přepnutí režimu na celou obrazovku, při spuštění nebo obnově emulace. Stále můžete přepnout na celou obrazovku pomocí alt-enter. Existující nastavení %s byly nalezeny v určeném adresáři nastavení. Chtěli byste tyto nastavení importovat nebo je přepsat výchozími hodnotami %s?

(nebo stiskněte Zrušit pro vybrání jiného adresáře nastavení) Selhání: Cílová paměťová karta '%s' se používá. Selhání: Kopírování je povoleno pouze na prázdnou pozici PS2 nebo do systému souborů. Opravy her můžou obejít špatnou emulaci v některých hrách.
Můžou ale také způsobit problémy s kompatibilitou a výkonem, takže nejsou doporučeny.
Opravy her jsou použity automaticky, takže zde nic nemusíte nastavovat. Známo, že ovlivňuje následující hry:
* Bleach Blade Battler
* Growlanser II and III
* Wizardry Známo, že ovlivňuje následující hry:
* Digital Devil Saga (Opravuje FMV a pády)
* SSX (Opravuje špatnou grafiku a pády)
* Resident Evil: Dead Aim (Způsobuje zkomolené textury) Známo, že ovlivňuje následující hry:
* Mana Khemia 1 (Going "off campus")
 Známo, že ovlivňuje následující hry:
* Test Drive Unlimited
* Transformers Komprese NTFS může být kdykoliv ručně změněna použitím vlastností souboru z Průzkumníku Windows. Komprimace NTFS je zabudovaná, rychlá a naprosto spolehlivá a většinou komprimuje paměťové karty velmi dobře (tato volba je vysoce doporučená). Nezapomeňte, že když je omezení snímků vypnuté, nebudou ani také dostupné režimy Turbo a ZpomalenýPohyb. Poznámka: Rekompilátory nejsou potřeba ke spuštění PCSX2, nicméně normálně výrazně zlepšují rychlost emulace. Možná budete muset ruřne rekompilátory znovu zapnout, pokud vyřešíte chyby. Upozornění: Kvůli hardwarovému designu PS2 je přesné přeskakování snímků nemožné. Zapnutím tohoto způsobí vážné grafické chyby v některých hrách. Upozornění: Většina her bude v pořádku s výchozím nastavením. Upozornění: Většina her bude v pořádku s výchozím nastavením. Došla Paměť (tak trochu): Rekompilátor SuperVJ nemohl vyhradit určitý vyžadovaný rozsah paměti a nebude dostupný k použití. To není kritická chyba, protože rek sVU je zastaralý a stejně byste místo něj měli používat mVU :). PCSX2 nemůže přidělit paměť potřebnou pro virtuální stroj PS2. Zavřete některé úlohy na pozadí náročné na paměť a zkuste to znovu. PCSX2 vyžaduje *legální* kopii BIOSu PS2, abyste mohli hrát hry.
Nemůžete použít kopii získanou od kamaráda nebo z Internetu.
Musíte ho vypsat z *Vaší* vlastní konzole Playstation 2. PCSX2 vyžaduje ke spuštění BIOS PS2. Z právních důvodů *musíte* BIOS získat ze skutečného PS2, které vlastníte (půjčení se nepočítá). Podívejte se prosím na Nejčastější Otázky a Průvodce pro další instrukce. Herní disky Playstation nejsou PCSX2 podporovány. Pokud chcete emulovat hry PSX, pak si budete muset stáhnout PSX emulátor, jako ePSXe nebo PCSX. Ujistěte se prosím, že tyto adresáře jsou vytvořeny a že Váš uživatelský účet má udělená oprávnění k zápisu do těchto adresářů -- nebo znovu spusťte PCSX2 jako správce (administrátorské oprávnění), což by mělo udělit PCSX2 schopnost samo si potřebné adresáře vytvořit. Pokud nemáte na tomto počítači správcovská oprávnění, pak budete muset přepnout do režimu Uživatelských Dokumentů (klikněte na tlačítko níže). Prosím zvolte platný BIOS. Pokud nejste schopni provést platnou volbu, pak stiskněte Zrušit pro zavření Konfiguračního panelu. Prosím níže vyberte vaše výchozí upřednostňované umístění pro dokumenty uživatelské úrovně PCSX2 (zahrnující paměťové karty, snímky obrazovky, nastavení a uložené stavy). Tato umístění adresářů mohou být kdykoli potlačena použitím panelu panelu výběru BIOSu/Zásuvných modulů. Má za cíl hlavně čekací smyčku EE na adrese 0x81FC0 v kernelu, tento hack se pokusí zjistit smyčky, jejichž těla mají zaručeně za následek stejný stav stroje pro každé opakování doku naplánovaná událost nespustí emulaci další jednotky. Po prvním opakováním takovýchto smyček, pokročíme do doby další události nebo konce pracovního intervalu procesoru, co nastane dříve. Odstraní jakýkoli šum výkonnostního testu způsobený vláknem VVGS nebo časem zpracování grafického procesoru. Tato volba se nejlépe používá spolu s uloženými stavy: uložte stav v ideální scéně, zapněte tuto volbu, a znovu načtěte uložený stav.

Varování: Tato volba může být zapnuta za běhu ale typicky nemůže být takto vypnuta (obraz bude většinou poškozený) Spouští VJ1 na svém vlastním vlákně (pouze mikroVJ1). Na počítačích s 3 a více jádry většinou zrychlení. Toto je pro většinu her bezpečné, ale některé jsou nekompatibilní a mohou se zaseknout. V případě her omezených GS může dojít ke zpomalení (zvláště na počítačích s dvoujádrovým procesorem). Nastavením vyšších hodnot na tomto šoupátku účinně sníží rychlost hodin jádra R5900 procesoru EmotionEngine a typicky přináší velké zrychlení hrám, které nemohou využívat plný potenciál skutečného hardwaru PS2.  Hacky Rychlosti většinou zlepšují rychlost emulace, ale můžou způsobovat chyby, špatný zvuk a špatné údaje o SZS. Když máte problémy s emulací, tento panel zakažte nejdříve. Pozice PS2 %d byla automaticky zakázána. Můžete tento problém opravit
a znovu ji kdykoli povolit pomocí Nastavení:Paměťové Karty z hlavního menu. Předvolby použijí hacky rychlosti, některá nastavení rekompilátoru a některé opravy her známé tím, že zvyšují rychlost.
Známé důležité opravy budou použity automaticky.

 --> Odškrtněte pro ruční změnu nastavení (se současnými předvolbami jako základ) Předvolby použijí hacky rychlosti, některá nastavení rekompilátoru a některé opravy her známé tím, že zvyšují rychlost.
Známé důležité opravy budou použity automaticky.

Informace o předvolbách:
1 --> Nejpřesnější emulace, ale také nejpomalejší.
3 --> Pokouší se vyvážit rychlost a kompatibilitu.
4 --> Některé agresivní hacky.
6 --> Příliš mnoho hacků, což pravděpodobně zpomalí většinu her.
 Zadaná cesta/adresář neexistuje. Chtěli byste je vytvořit? Vlákno '%s' neodpovídá. Mohlo uváznout, nebo prostě běží *velmi* pomalu. Není dostatek virtuální paměti, nebo potřebná mapování virtuální paměti již byly vyhrazeny jinými procesy, službami, nebo DLL. Tato činnost resetuje existující stav virtuálního stroje PS2; veškerý současný postup bude ztracen. Jste si jisti? Tento příkaz vyčistí nastavení %s a umožňuje Vám znovu spustit Průvodce Prvním Spuštěním. Po této operaci budete muset ručně restartovat %s.

VAROVÁNÍ!! Kliknutím na OK smažete *VŠECHNA* nastavení pro %s a přinutíte tuto aplikaci uzavřít, čímž ztratíte jakýkoli postup emulace. Jste si naprosto jisti?

(poznámka: nastavení zásuvných modulů nejsou ovlivněna) Do tohoto adresáře PCSX2 ukládá uložené stavy, které jsou zaznamenány buď použitím menu/panelů nástrojů, nebo stisknutím F1/F3 (uložit/nahrát). Toto je adresář, kde PCSX2 ukládá své soubory se záznamem a diagnostické výpisy. Většina zásuvných modulů bude také používat tento adresář, ale některé starší ho můžou ignorovat. Toto je adresář, kde PCSX2 ukládá snímky obrazovky. Vlastní formát a styl snímku se může měnit v závislosti na používaném zásuvném modulu GS. Tento hack funguje nejlépe v hrách, které používají stavy KPŘE registru pro čekání na vsynch, což hlavně zahrnuje ne-3D rpg hry. Ty, co tuto metodu v synch nepoužívají z tohoto hacku nedostanou žádné nebo malé zrychlení. Do tohoto adresáře PCSX2 ukládá Vaše nastavení, zahrnující i nastavení vytvořená většinou zásuvných modulů (některé starší moduly nemusí tuto hodnotu respektovat). Tento rekompilátor nemohl vyhradit přilehlou paměť potřebnou pro vnitřní vyrovnávací paměti. Tato chyba může být způsobena nízkými zdroji virtuální paměti, jako např. vypnutý nebo malý stránkovací soubor, nebo jiným programem náročným na paměť. Toto šoupátko kontroluje množství cyklů, které VJ ukradne od EmotionEngine. Vyšší hodnoty zvyšují počet ukradených cyklů od EE pro každý mikroprogram, který VJ spustí. Tento průvodce Vám pomůže skrz nastavení zásuvných modulů, paměťových karet a BIOSu. Je doporučeno, pokud je toto poprvé co instalujete %s, si prohlédnout 'Přečti mě' a průvodce nastavením. Aktualizuje Příznaky Stavu pouze v blocích, které je budou číst, místo neustále. Toto je většinou bezpečné a Super VJ dělá standardně něco podobného. Vsynch odstraňuje trhání obrazovky, ale má velký vliv na výkon. Většinou se toto týká režimu celé obrazovky a nemusí fungovat se všemi zásuvnými moduly GS. Varování! Změna zásuvných modulů vyžaduje celkové vypnutí a reset virtuálního stroje PS2. PCSX2 se pokusí stav uložit a obnovit, ale pokud jsou nově zvolené zásuvné moduly nekompatibilní, obnovení může selhat a současný postup může být ztracen.

Jste si jisti, že chcete nastavení použít teď? Varování! Spouštíte PCSX2 s volbami příkazového řádku, které potlačují Vaše uložená nastavení zásuvných modulů a/nebo adresářů. Tyto volby příkazového řádku se nebudou odrážet v dialogovém okně Nastavení a budou zrušeny, když zde použijete jakékoli změny nastavení. Varování! Spouštíte PCSX2 s volbami příkazového řádku, které potlačují Vaše uložená nastavení. Tyto volby příkazového řádku se nebudou odrážet v dialogovém okně Nastavení a budou zrušeny, pokud zde použijete jakékoli změny. Varování: Některé z nastavených rekompilátorů PS2 nelze spustit a byly zakázány: Varování: Váš počítač nepodporuje SSE2, která je vyžadována většinou rekompilátorů PCSX2 a zásuvných modulů. Vaše volby budou omezené a emulace bude *velmi* pomalá. Je-li zaškrtnuto, tento adresář bude automaticky odrážet výchozí asociaci se současným nastavením uživatelského režimu PCSX2. Chystáte se smazat formátovanou paměťovou kartu '%s'. Všechna data na kartě budou ztracena! Jste si naprosto a zcela jisti? Prosím vyberte níže Vaši upřednostňované výchozí umístění pro dokumenty uživatelské úrovně PCSX2 (zahrnující paměťové karty, snímky obrazovky, nastavení a uložené stavy). Tato volba ovlivňuje pouze Standardní Cesty, které jsou nastaveny, aby používali výchozí hodnoty instalace. Můžete také zde dobrovolně zadat umístění Vašeho nastavení PCSX2. Pokud umístění obsahuje existující nastavení PCSX2, bude Vám dána možnost je importovat nebo přepsat. Váš systém má příliš nízké virtuální zdroje, aby mohl být PCSX2 spuštěn. To může být způsobeno malým nebo vypnutým stránkovacím souborem, nebo jinými programy, které jsou náročné na zdroje. Přiblížení = 100: Celý obraz bude umístěn do okna bez jakéhokoliv oříznutí.
Nad/Pod 100: Přiblížení/Oddálení
0: Automaticky přibližovat, dokud černé čáry nezmizí (poměr stran je zachován, část obrazu bude mimo obrazovku).
POZNÁMKA: Některé hry vykreslují vlastní černé čáry, které pomocí '0' nebudou odstraněny.

Klávesnice: CTRL + PLUS: Přiblížení, CTRL + MÍNUS: Oddálení, CTRL + HVĚZDIČKA: Přepínání 100/0. 