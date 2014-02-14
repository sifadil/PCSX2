��    H      \  a   �         �   !  9   �  Y   �  R   ?  b   �  b   �  w   X  o   �  �   @	  �   
  ~   �
  �   *  `   �  Q   R  �  �  �   L  �   �  /   �  M   �  &  4  ]   [  �   �  G   e  H   �  d   �  �   [  b   �  �   P  �      5   �  6   �  �     �     �   �  �   S  �   )  r  �  |   >  �  �  N  H  �   �  �   �  �   t   �   !  �   �!  z  �"  J   $  h   Z$  �   �$  p   X%  G  �%  �   '  �   �'  �   2(  �   �(  �   �)  �   &*  �   �*  �   �+  �   H,  *  �,  �   .  �   
/  \   �/  �   G0  r   �0  �   a1  �   �1  �   �2  �   �3  �  74  �  �5  �   �7  2   )8  D   \8  l   �8  r   9  o   �9  w   �9  �   i:  �   �:  �   �;  �   q<  �   �<  ]   �=  o   K>  �  �>  �   �@  �   UA  2   B  Q   QB  4  �B  ^   �C  �   7D  I   �D  J   GE  u   �E  �   F  l   �F  
  �F  �   H  ;   �H  ;   �H  �   (I  x   J  �   �J  �   JK  �   �K  T  �L  n   �M  Y  IN  3  �O  =  �P  �   R  �   �R  �   �S    LT  �  SU  H   W  d   LW  {   �W  u   -X  k  �X  �   Z  �   �Z  �   K[  �   �[  �   �\  �   O]  �   �]  �   �^  �   >_  4  �_  �   $a  �   b  ^   �b  �   =c  e   �c  Q   Od  �   �d  �   �e  �   Ff  �  g                          	   :   A   -         6      ;   *   1               2   @   7   5          B             !   $   .      F       G   C   %   "                     4   <   (   D       E   ?       '      #             3          9       =      0   /                              ,       8                    H          )   >   
   &             +        'Ignore' to continue waiting for the thread to respond.
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
You must dump the BIOS from your *own* Playstation 2 console. PCSX2 requires a PS2 BIOS in order to run.  For legal reasons, you *must* obtain a BIOS from an actual PS2 unit that you own (borrowing doesn't count).  Please consult the FAQs and Guides for further instructions. Playstation game discs are not supported by PCSX2.  If you want to emulate PSX games then you'll have to download a PSX-specific emulator, such as ePSXe or PCSX. Please ensure that these folders are created and that your user account is granted write permissions to them -- or re-run PCSX2 with elevated (administrator) rights, which should grant PCSX2 the ability to create the necessary folders itself.  If you do not have elevated rights on this computer, then you will need to switch to User Documents mode (click button below). Please select a valid BIOS.  If you are unable to make a valid selection then press Cancel to close the Configuration panel. Primarily targetting the EE idle loop at address 0x81FC0 in the kernel, this hack attempts to detect loops whose bodies are guaranteed to result in the same machine state for every iteration until a scheduled event triggers emulation of another unit.  After a single iteration of such loops, we advance to the time of the next event or the end of the processor's timeslice, whichever comes first. Removes any benchmark noise caused by the MTGS thread or GPU overhead.  This option is best used in conjunction with savestates: save a state at an ideal scene, enable this option, and re-load the savestate.

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

(note: settings for plugins are unaffected) This folder is where PCSX2 records savestates; which are recorded either by using menus/toolbars, or by pressing F1/F3 (save/load). This folder is where PCSX2 saves its logfiles and diagnostic dumps.  Most plugins will also adhere to this folder, however some older plugins may ignore it. This folder is where PCSX2 saves screenshots.  Actual screenshot image format and style may vary depending on the GS plugin being used. This hack works best for games that use the INTC Status register to wait for vsyncs, which includes primarily non-3D RPG titles. Games that do not use this method of vsync will see little or no speedup from this hack. This is the folder where PCSX2 saves your settings, including settings generated by most plugins (some older plugins may not respect this value). This slider controls the amount of cycles the VU unit steals from the EmotionEngine.  Higher values increase the number of cycles stolen from the EE for each VU microprogram the game runs. This wizard will help guide you through configuring plugins, memory cards, and BIOS.  It is recommended if this is your first time installing %s that you view the readme and configuration guide. Updates Status Flags only on blocks which will read them, instead of all the time. This is safe most of the time, and Super VU does something similar by default. Vsync eliminates screen tearing but typically has a big performance hit. It usually only applies to fullscreen mode, and may not work with all GS plugins. Warning!  Changing plugins requires a complete shutdown and reset of the PS2 virtual machine. PCSX2 will attempt to save and restore the state, but if the newly selected plugins are incompatible the recovery may fail, and current progress will be lost.

Are you sure you want to apply settings now? Warning!  You are running PCSX2 with command line options that override your configured plugin and/or folder settings.  These command line options will not be reflected in the settings dialog, and will be disabled when you apply settings changes here. Warning!  You are running PCSX2 with command line options that override your configured settings.  These command line options will not be reflected in the Settings dialog, and will be disabled if you apply any changes here. Warning: Some of the configured PS2 recompilers failed to initialize and have been disabled: Warning: Your computer does not support SSE2, which is required by many PCSX2 recompilers and plugins. Your options will be limited and emulation will be *very* slow. When checked this folder will automatically reflect the default associated with PCSX2's current usermode setting.  You are about to delete the formatted memory card '%s'. All data on this card will be lost!  Are you absolutely and quite positively sure? You can change the preferred default location for PCSX2 user-level documents here (includes memory cards, screenshots, settings, and savestates).  This option only affects Standard Paths which are set to use the installation default value. You may optionally specify a location for your PCSX2 settings here.  If the location contains existing PCSX2 settings, you will be given the option to import or overwrite them. Your system is too low on virtual resources for PCSX2 to run. This can be caused by having a small or disabled swapfile, or by other programs that are hogging resources. Zoom = 100: Fit the entire image to the window without any cropping.
Above/Below 100: Zoom In/Out
0: Automatic-Zoom-In untill the black-bars are gone (Aspect ratio is kept, some of the image goes out of screen).
  NOTE: Some games draw their own black-bars, which will not be removed with '0'.

Keyboard: CTRL + NUMPAD-PLUS: Zoom-In, CTRL + NUMPAD-MINUS: Zoom-Out, CTRL + NUMPAD-*: Toggle 100/0 Project-Id-Version: PCSX2 0.9.9
Report-Msgid-Bugs-To: http://code.google.com/p/pcsx2/
POT-Creation-Date: 2014-02-01 18:19+0100
PO-Revision-Date: 2012-05-23 10:01+0800
Last-Translator: 
Language-Team: kohaku2421 <kohaku2421@gmail.com>
Language: 
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
X-Poedit-KeywordsList: pxE;pxEt
X-Poedit-SourceCharset: utf-8
X-Poedit-Basepath: trunk\
X-Poedit-SearchPath-0: pcsx2
X-Poedit-SearchPath-1: common
 'Abaikan' utk terus menuggu thread utk memberi respon.
'Batal' utk cuba membatalkan thread.
'Henti' utk menutup PCSX2 serta-merta.
 Mematikan VU Cycle Stealing. Seting paling stabil! Cyclerate asal. Ini sgt menyamai kelajuan EmotionEngine PS2 sebenar. Sedikit VU Cycle Stealing. Merendahkan kestabilan, tetapi sedikit pertambahan kelajuan bagi kebanyakan game. VU Cycle Stealing sederhana. Merendahkan lagi kestabilan, tetapi sangat meningkatkan kelajuan dlm sesetengah game. Mengurangkan cyclerate EE sebanyak 33%. Sedikit pertambahan kelajuan utk kebanyakan game dgn kestabilan tinggi. VU Cycle Stealing maksimum. Kegunaannya terhad, kerana ia akan menyebabkan flickering atau slowdown dlm kebayakan game. Mengurangkan cyclerate EE sebanyak 50%. Lebih banyak pertambahan kelajuan, tetapi *akan* menyebabkan stuttering audio dlm banyak FMV. Semua plugin mesti mempunyai pilihan yg sah utk %s dijalankan. Jika anda gagal utk membuat pilihan yg sah kerana kehilangan plugin atau pemasangan %s yg tidak lengkap, maka tekan Batal utk menutup panel Kofigurasi. Mengelakkan kad memori dprd korup dgn memaksa games mengindeks semula kandungan kad selepas memuat dari savestate. Mungkin tidak sesuai dgn semua permainan (Guitar Hero). Semak HDLoader utk senarai kesesuaian game-game yg mempunyai masalah dgn hack ini. (Sering ditandakan sbg memerlukan 'mode 1' atau 'slow DVD' Tandakan ini utk memaksa kursor tetikus utk tidak ditunjukkan dlm tetingkap GS; berguna jika meggunakan tetikus sbg controller utama semasa bermain. Pada asalnya kursor tetikus akan sembunyi sendiri selepas ketidak aktifan selama 2 saat. Menutup tetingkap GS sepenuhnya yg selalunya besar apabila menekan ESC atau menjeda emulator. Utk trubleshooting kemungkinan pepijat yg mungkin terdapat dlm MTGS sahaja, kerana ia mungkin sangat perlahan.  Menghidupkan Vsync apabila framerate betul-betul pada kelajuan penuh. Kurang shj drpd kelajuan tersebut, Vsync akan dimatikan utk mengelakkan impak prestasi yg lebih besar. Nota: Pada masa ini ia hanya berfungsi baik dgn GSdx sbg plugin GS dan di konfigurasi utk menggunakan DX10/11 hardware rendering. Plugin-plugin atau mod rendering lain akan mengabaikannya atau menghasilkan frame hitam yg berkelip apabila mod ini berfungsi. Ia juga memerlukan Vsync untuk diaktifkan. Menghidupkan penukaran mod secara automatik kepada fullscreen apabila memulakan atau menyambung semula emulation. Anda masih boleh menukar paparan fullscreen pada bila-bila masa dgn alt-enter. Seting sedia ada %s tlh dijumpai dlm folder tetapan yg tlh di konfigurasi. Adakah anda mahu import seting ini atau buat yg baru dgn nilai asal %s?

(atau tekan Batal utk memilih folder seting yg lain) Gagal: Destinasi kad memori '%s' sedang digunakan. Gagal: Duplicate hanya dibenarkan kepada port-PS2 kosong atau kepada fail sistem. GameFix akan menyebabkan emulation salah dalam sesetengah game. 
Ia juga boleh menyebabkan masalah kestabilan dan kelajuan. 

Adalah lebih baik utk mneghidupkan 'GameFixes Automatik' dlm menu utama, dan membiarkan halaman ini kosong. 
 ('Automatik' bermaksud: memberi game yg tertentu dgn fix yg bersesuaian) Diketahui memberi kesan pada game:
 * Bleach Blade Battler
 * Growlanse II dan III
 * Wizardry Diketahui akan memberi kesan pada game:
 * Digital Devil Saga (membetulkan FMV dan crash) 
 * SSX (membetulkan grafik pelik dan crash) 
 * Resident Evil: Dead Aim (menyebabkan tekstur pelik/hancur) Diketahui memberi kesan pada game:
 * Mana Khemia 1 (Pergi "off campus")
 Diketahui memberi kesan pada game:
 * Test Drive Unlimited
 * Transformers Pemampatan NTFS boleh diubah secara manual pada bila-bila masa dgn menggunakan file properties dari Windows Explorer. Mampatan NTFS adalah terbina dalam, pantas, dan sgt berguna; digunakan utk memampat kad memori dgn baik (opsyen ini sgt digalakkan). Sila ambil tahu bahawa apabila Framelimiting dimatikan, mod Turbo dan SlowMotion juga akan tidak dibenarkan. Nota: Recompiler tidak diperlukan oleh PCSX2 utk dijalankan, walau bagaimanapun ia penting dlm meningkatkan kelajuan emulation. Anda kemungkinan perlu menghidupkan semula secara manual recompiler dlm senarai di atas, jika anda dapat menyelesaikan masalah anda kelak. Notis: Oleh kerana design perkakasan PS2, frame skipping yg tepat adalah mustahil. Menghidupkannnya hanya akan menyebabkan ralat grafik yg agak teruk dlm sesetengah game. Notis: Kebanyakan game berfungsi dgn baik pada opsyen asal. Notis: Kebanyakan game berfungsi dgn baik pada opsyen asal. Kekurangan Memori: Recompiler SuperVU gagal untuk menempah julat memori yg diperlukan, dan tidak akan ada utk digunakan. Ralat ini tidak bersifat kritikal, kerana sVU rec telah menjadi usang, dan anda patut menggunakan microVU. :) PCSX2 gagal utk menempah memori utk PS2 virtual machine. Tutup aplikasi yg banyak menggunakan memori kemudian cuba lagi. PCSX2 memerlukan salinan BIOS yg *sah* drpd PS2 utk menjalankan permainan.
Anda tidak boleh menggunakan salinan yg didapati drpd rakan atau Internet.
Anda mesti dump BIOS drpd PS2 anda sendiri. PCSX2 memerlukan PS2 BIOS utk dijalankan. Disebabkan hakcipta, anda mesti *MENDAPATKAN* BIOS dari PS2 MILIK ANDA SENDIRI. Sila rujuk FAQ dan panduan utk maklumat lanjut. Playstation aka PS1 disc tidak disokong oleh PCSX2. Jika anda nak emulate PS1, maka anda kena download emulator PS1, contohnye, ePSXe atau PCSX. Sila pastikan folder-folder ini tlh dibuat dan akaun pengguna anda mempunyai hak menulis ke atas mereka -- atau jalankan semula PCSX2 dgn hak administrator, dimana ia membolehkan PCSX2 utk membuat folder sendiri. Jika anda tidak mempunyai hak terhadap komputer ini, maka anda perlu menukar kepada User Documents Mode (klik butang di bawah). Sila pilih BIOS yg sah. Jika anda gagal melakukannya maka tekan Batal utk menutup tetingkap panel Konfigurasi. Target utamanya adalah idle loop EE pada alamat 0x81FC0 dlm kernel, hack ini akan detect loop yg dipastikan akan memberi hasil machine state yg sama hingga acara yg dijadualkan trigger emulation dlm unit lain. Selepas satu loop tersebut, kita mempercepatkan masa untuk acara seterusnya atau penghujung masa pemproses, mana-mana yg datang dahulu. Membuang semua noise yg dihasilkan oleh thread MTGS atau overhead GPU. Opsyen ini bagus digunakan bersama savestate: simpan sebuah state, hidupkan opsyen ini, dan muat semula savestate.

Amaran: Opsyen ini boleh dihidupkan on-the-fly tetapi tidak boleh dimatikan on-the-fly (video akan menjadi buruk/pelik). Menjalankan VU1 pada threadnya sendiri (microVU1-sahaja). Selalunya akan meningkatkan kelajuan pada CPU yg mempunyai 3 teras atau lebih. Seting ini selamat bagi kebanyakan game, tatapi sesetengahnya yg tidak sesuai akan hang. Dalam kes game yg dihadkan GSnya, ia akan menyebabkan slowdown (terutamanya dual core CPU). Menetapkan nilai yg lebih tinggi akan mengurangkan clock speed teras cpu EmotionEngine R5900 dgn efektif, dan akan memberi banyak peningkatan kelajuan pada game yg gagal menggunakan potensi penuh perkakasan sebenar PS2. Speedhack selalunya meningkatkan prestasi emulation, tetapi akan menyebabkan glitch, audio pelik, dan bacaan FPS palsu. Apabila menghadapi masalah emulation, matikan panel ini dahulu. Slot-PS2 %d telah diamtikan secara automatik. Anda boleh membetulkan masalah
dan menghidupkannya semula pada bila-bila masa dgn Konfig:Kad Memori drpd menu utama. Preset akan menetapkan speed hack, sesetengah opsyen recompiler dan game fix yg diketahui akan meningkatkan kelajuan.
Game fix penting yg diketahui akan ditetapkan secara automatik.

--> Jgn tanda utk mengubah seting secara manual (dgn seting sekarang sbg tapak) Preset akan menetapkan speed hack, sesetengah opsyen recompiler dan game fix yg diketahui akan meningkatkan kelajuan.
Game fix penting yg diketahui akan ditetapkan secara automatik.

Info Preset:
1 --> Emulation yg paling tepat tetapi paling perlahan.
3 --> Cuba utk menetapkan ketepatan emulation dan kestabilan.
4 --> Sedikit hack-hack agresif.
6 --> Terlalu banyak hack yg berkemungkinan akan menyebabkan game menjadi perlahan.
 Laluan/Direktori yg dinyatakan tidak wujud. Adakah anda mahu membuatnya? Thread %s tidak memberi respon. Mungkin ia tlh dikunci, atau dijalankan dgn kelajuan *sgt* perlahan. Memori maya tidak mencukupi, atau pengalamatan memori maya yang diperlukan telah ditempah oleh proses lain, servis dan DLL. Perbuatan ini akan reset state virtual machine PS2 yg sedia ada; semua perkembangan sekarang akan hilang. Anda pasti? Arahan ini akan mengosongkan seting %s dan membolehkan anda menjalankan Wizard Kali Pertama. Anda juga perlu membuka semula %s secara manual selepas operasi ini.

AMARAN!! Tekan OK utk buang *SEMUA* seting utk %s dan memaksa aplikasi utk ditutup, juga menghilangkan apa-apa perkembangan emulation. Anda benar-benar pasti?

(nota: tidak termasuk seting utk plugin) Folder ini adalah dimana PCSX2 merekod savestate; direkod samada menggunakan menu/toolbar, atau dgn menekan F1/F3 (simpan/muat). Folder ini adalah dimana PCSX2 menyimpan fail log dan dump diagnostik. Kebanyakan plugin akan turut menggunakan folder ini, walau bagaimanapun sesetengah plugin lama akan mengabaikannya. Folder ini adalah dimana PCSX2 menyimpan screenshot. Format imej scrrenshot dan stailnya akan berbeza bergantung kepada plugin GS yg digunakan. Hack ini bagus utk game yg menggunakan INTC Status register untuk menunggu Vsync, dimana selalunya terdapat pada game bkn RPG. Game yg tidak menggunakan teknik vsync ini akan mendapat sedikit atau tiada pertambahan kelajuan. Ini adalah folder dimana PCSX2 menyimpan tetapan anda termasuk seting yg dihasilkan oleh kebanyakan plugin (sesetengah plugin lama mungkin tidak). Slider ini mengawal amaun cycle curian VU unit dari EmotionEngine. Nilai yg lebih tinggi menambah cycle curian drpd EE bg setiap microprogram VU yg dijalankan game. Wizard akan membantu dlm men-konfigurasi plugin, kad memori, dan BIOS. Jika ini kali pertama anda menggunakan %s, adalah digalakkan anda utk membaca readme panduan konfigurasi. Megemaskini Status Flag pada blok yg akan membacanya sahaja, drpd sepanjang masa. Seting ini selamat dan SuperVU melakukan perkara yg sama pada asalnya. Vsync menghilangkan screen tearing tetapi akan memberi impak pada kelajuan. Ia selalunya digunakan dlm mod fullscreen, dan kemungkinan tidak akan berfungsi dgn semua plugin GS. Amaran! Penukaran plugin memerlukan penutupan lengkap dan reset kepada virtual machine PS2. PCSX2 akan cuba utk menyimpan dan mengembalikan semula state, tetapi jika plugin yg baru dipilih tidak sesuai, pemulihan semula akan gagal, dan segala progress akan hilang.

Anda pasti utk menetapkan seting sekarang? Amaran! Anda menjalankan PCSX2 dgn opsyen command line yg mengambil alih seting asal anda. Opsyen command line tidak dinyatakan dlm kotak dialog tetapan, dan akan dimatikan jika anda menetapkan apa-apa perubahan di sini. Amaran! Anda menjalankan PCSX2 dgn opsyen command line yg mengambil alih seting asal anda. Opsyen command line tidak dinyatakan dlm kotak dialog tetapan, dan akan dimatikan jika anda menetapkan apa-apa perubahan di sini. Amaran: Sesetengah PS2 recompiler yg tlh di konfigurasi gagal utk dimulakan dan tlh dimatikan. Amaran: Komputer anda tidak menyokong SSE2, dimana ia diperlukan oleh PCSX2 recompiler dan plugin. Pilihan anda akan menjadi terhad dan emulation akan jadi *sgt* perlahan. Apabila ditanda, folder ini akan mengikut yg asalnya terlibat dgn seting mod pengguna PCSX2 sekarang. Anda akan membuang kad memori %s. Semua data dlm kad ini akan hilang! Anda pasti? Anda boleh mengubah folder asal utk dokumen level pengguna PCSX2 (termasuk kad memori, screenshot, seting dan savestate). Opsyen ini hanya memberi kesan pada Laluan Standard yg tlh di set utk menggunakan nilai asal semasa pemasangan. Anda tidak di wajibkan utk menyatakan tempat utk tetapan PCSX2 disini. Jika tempat tersebut sedia ada mengandungi tetapan PCSX2, anda akan diberi pilihan utk import atau membuat yg baru. Sistem anda kekurangan memori/ruang maya untuk PCSX2 dijalankan. Ini boleh disebabkan oleh swapfile yg kecil atau dimatikan, atau program lain yg menggunakan terlalu banyak memori/ruang. Zoom = 100: Memuatkan seluruh imej kedalam kotak tetingkap tanpa ruang kosong.
Atas/bawah 100: Zoom Masuk/Keluar
0: Zoom Masuk secara automatik hingga bar hitam tiada (aspect ratio dikekalkan, sesetengah imej akan terkeluar dari skrin).
  NOTA: Sesetengah game akan melukis bar hitam mereka sendiri, dimana tidak dihilangkan dengan '0'.

Keyboard: CTRL + NUMPAD-PLUS: Zoom Masuk, CTRL + NUMPAD-MINUS: Zoom Keluar, CTRL + NUMPAD-*: Tukar antara 100/0 