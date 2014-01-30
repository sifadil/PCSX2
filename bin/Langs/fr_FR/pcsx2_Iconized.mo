��    D      <  a   \      �  �   �  9   k  Y   �  R   �  b   R  b   �  w     o   �  �    	  �   �	  ~   k
  �   �
  `   �  Q     �  d  �     M   �  &  �  ]     �   u  G   !  H   i  d   �  �     b   �  �     �   �  5   d  6   �  �   �  �   �  �   N  �     �   �  r  �  |   �  �  w  N    �   S  �   S  �   0  �   �  �   n   z  P!  J   �"  h   #  �   #  p   $  G  �$  �   �%  �   Q&  �   �&  �   v'  �   P(  �   �(  �   �)  �   A*  *  �*  �   ,  �   -  \   �-  �   @.  r   �.  �   Z/  �   J0  �   �0  �  �1  *  03  �   [5  L   
6  u   W6  y   �6  �   G7  m   �7  �   =8  �   �8  �   Z9  �   U:  �   6;    �;  `   =  m   n=  �  �=  �   �?  U   �@  �  A  �   �B  �   PC  m   D  [   {D  �   �D  e   \E  ^   �E    !F  �   ;G  _   �G  _   OH    �H  �   �I  
  �J    �K  �   �L  �  jM  �   O  ;   �O  �  �O  f  �Q  �   S    T  �   U  z  �U     aW  T   �Y  l   �Y  �   DZ  �   �Z    �[  �   �]  �   .^  �   �^  �   �_  �   �`  �   a  �   �a    �b  e  �c  �   De  �   =f  �   .g  �   �g  j   �h  �   i  �   �i  �   �j  �  7k                          	   8   >   +         4          (   /               0   =   5   3          ?                "   ,      B       C   @   #                         2   9   &           A   <       %      !             1         7       :      .   -                               *       6                    D          '   ;   
   $              )        'Ignore' to continue waiting for the thread to respond.
'Cancel' to attempt to cancel the thread.
'Terminate' to quit PCSX2 immediately.
 0 - Disables VU Cycle Stealing.  Most compatible setting! 1 - Default cyclerate. This closely matches the actual speed of a real PS2 EmotionEngine. 1 - Mild VU Cycle Stealing.  Lower compatibility, but some speedup for most games. 2 - Moderate VU Cycle Stealing.  Even lower compatibility, but significant speedups in some games. 2 - Reduces the EE's cyclerate by about 33%.  Mild speedup for most games with high compatibility. 3 - Maximum VU Cycle Stealing.  Usefulness is limited, as this will cause flickering visuals or slowdown in most games. 3 - Reduces the EE's cyclerate by about 50%.  Moderate speedup, but *will* cause stuttering audio on many FMVs. All plugins must have valid selections for %s to run.  If you are unable to make a valid selection due to missing plugins or an incomplete install of %s, then press Cancel to close the Configuration panel. Avoids memory card corruption by forcing games to re-index card contents after loading from savestates.  May not be compatible with all games (Guitar Hero). Check HDLoader compatibility lists for known games that have issues with this. (Often marked as needing 'mode 1' or 'slow DVD' Check this to force the mouse cursor invisible inside the GS window; useful if using the mouse as a primary control device for gaming.  By default the mouse auto-hides after 2 seconds of inactivity. Completely closes the often large and bulky GS window when pressing ESC or pausing the emulator. Enable this if you think MTGS thread sync is causing crashes or graphical errors. Enables Vsync when the framerate is exactly at full speed. Should it fall below that, Vsync gets disabled to avoid further performance penalties. Note: This currently only works well with GSdx as GS plugin and with it configured to use DX10/11 hardware rendering. Any other plugin or rendering mode will either ignore it or produce a black frame that blinks whenever the mode switches. It also requires Vsync to be enabled. Enables automatic mode switch to fullscreen when starting or resuming emulation. You can still toggle fullscreen display at any time using alt-enter. Failed: Duplicate is only allowed to an empty PS2-Port or to the file system. Gamefixes can work around wrong emulation in some titles. 
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

(note: settings for plugins are unaffected) This folder is where PCSX2 records savestates; which are recorded either by using menus/toolbars, or by pressing F1/F3 (save/load). This folder is where PCSX2 saves its logfiles and diagnostic dumps.  Most plugins will also adhere to this folder, however some older plugins may ignore it. This folder is where PCSX2 saves screenshots.  Actual screenshot image format and style may vary depending on the GS plugin being used. This hack works best for games that use the INTC Status register to wait for vsyncs, which includes primarily non-3D RPG titles. Games that do not use this method of vsync will see little or no speedup from this hack. This is the folder where PCSX2 saves your settings, including settings generated by most plugins (some older plugins may not respect this value). This slider controls the amount of cycles the VU unit steals from the EmotionEngine.  Higher values increase the number of cycles stolen from the EE for each VU microprogram the game runs. Updates Status Flags only on blocks which will read them, instead of all the time. This is safe most of the time, and Super VU does something similar by default. Vsync eliminates screen tearing but typically has a big performance hit. It usually only applies to fullscreen mode, and may not work with all GS plugins. Warning!  Changing plugins requires a complete shutdown and reset of the PS2 virtual machine. PCSX2 will attempt to save and restore the state, but if the newly selected plugins are incompatible the recovery may fail, and current progress will be lost.

Are you sure you want to apply settings now? Warning!  You are running PCSX2 with command line options that override your configured plugin and/or folder settings.  These command line options will not be reflected in the settings dialog, and will be disabled when you apply settings changes here. Warning!  You are running PCSX2 with command line options that override your configured settings.  These command line options will not be reflected in the Settings dialog, and will be disabled if you apply any changes here. Warning: Some of the configured PS2 recompilers failed to initialize and have been disabled: Warning: Your computer does not support SSE2, which is required by many PCSX2 recompilers and plugins. Your options will be limited and emulation will be *very* slow. When checked this folder will automatically reflect the default associated with PCSX2's current usermode setting.  You can change the preferred default location for PCSX2 user-level documents here (includes memory cards, screenshots, settings, and savestates).  This option only affects Standard Paths which are set to use the installation default value. You may optionally specify a location for your PCSX2 settings here.  If the location contains existing PCSX2 settings, you will be given the option to import or overwrite them. Your system is too low on virtual resources for PCSX2 to run. This can be caused by having a small or disabled swapfile, or by other programs that are hogging resources. Zoom = 100: Fit the entire image to the window without any cropping.
Above/Below 100: Zoom In/Out
0: Automatic-Zoom-In untill the black-bars are gone (Aspect ratio is kept, some of the image goes out of screen).
  NOTE: Some games draw their own black-bars, which will not be removed with '0'.

Keyboard: CTRL + NUMPAD-PLUS: Zoom-In, CTRL + NUMPAD-MINUS: Zoom-Out, CTRL + NUMPAD-*: Toggle 100/0 Project-Id-Version: PCSX2 0.9.9
Report-Msgid-Bugs-To: http://code.google.com/p/pcsx2/
POT-Creation-Date: 2013-12-15 13:24+0100
PO-Revision-Date: 2012-03-08 11:33-0000
Last-Translator: goldeng <odakawoi@yahoo.fr>
Language-Team: goldeng <odakawoi@yahoo.fr>
Language: 
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
X-Poedit-KeywordsList: pxE;pxEt
X-Poedit-SourceCharset: utf-8
X-Poedit-Basepath: trunk\
X-Poedit-Language: French
X-Poedit-Country: France
X-Poedit-SearchPath-0: pcsx2
X-Poedit-SearchPath-1: common
 Cliquez sur Ignorer pour attendre jusqu'à ce que le processus réponde.
Cliquez Annuler pour mettre fin au processus.
Cliquez sur Terminer pour fermer immédiatement PCSX2.
 0 - Désactive le VU Cycle Stealing : compatibilité maximale, évidemment ! 1 - Cyclerate par défaut : la vitesse d'émulation est équivalente à celle d'une véritable console Playstation 2. 1 - VU Cycle Stealing léger : faible compatibilité mais une amélioration sensible des performances pour certains jeux. 2 - VU Cycle Stealing modéré : très faible compatibilité, mais une amélioration significative des performances pour certains jeux. 2 - Diminue l'EE Cyclerate d'envion 33% : amélioration sensible des performances et compatibilité élevée. 3 - VU Cycle Stealing maximal : relativement inutile puisqu'il engendre des ralentissements et des bugs graphiques dans la plupart des jeux. 3 - Diminue l'EE Cyclerate d'environ 50% : amélioration modérée des performances, mais le son de certaines cinématiques sera insupportable. Tous les plugins doivent être corrects pour lancer %s. Si vous n'êtes pas capabled'obtenir une seélection valide à cause de plugins manquants ou d'une installation incomplète de %s alors cliquer sur Annuler pour fermer le paneau de Configuration Permet d'éviter d'endommager la carte mémoire (fichiers corrompus) en obligeant les jeux à ré-indexer le contenu présent sur la carte.
Cette option peut être incompatible avec certains jeux (Guitar Hero, par exemple) ! Vérifie les listes de compatibilité du HDLoader pour les jeux qui rencontrent habituellement des problèmes avec lui (le plus souvent, repérables par l'indication "mode 1" ou "slow DVD"). Cochez cette case pour obliger le curseur de la souris à ne pas apparaître dans la fenêtre de jeu.
Cette option est utile si vous utilisez la souris comme contrôleur principal pour les jeux.
Par défaut, le curseur s'efface automatiquement après deux secondes d'inactivité. Ferme complètement la fenêtre de jeu lorsque la touche ESC ou la fonction Pause est utilisée. Activez cette option si vous pensez que le processus SyncMTGS cause des bugs graphiques ou crashs récurents. Active la Vsync (synchronisation verticale) lorsque la framerate atteint sa vitesse maximum. Si cette dernière venait à diminuer, la Vsync se désactivera automatiquement.

Attention : En l'état actuel, cette option ne fonctionne qu'avec le plugin GSdx dans sa configuration DX10/11 (hardware) ! Tout autre plugin ou mode de rendu occasionnera l'affichage d'un fond noir à la place des éléments graphiques du jeu.
De plus, cette option nécessite que la Vsync soit activée sur votre PC. Lorsque cette option est activée, le jeu se lance automatiquement en mode plein-écran (au démarrage ou après une pause).
Vous pouvez néanmoins modifier le mode d'affichage en jeu grâce à la combinaison de touches ALT + Entrée. Erreur : Le port-PS2 concerné est occupé par un autre service, empêchant la copie. Les patchs peuvent corriger des problèmes liés à l'émulation de titres spécifiques.
Néanmoins, ils peuvent aussi avoir un impact sur la compatibilité ou les performances globales.

Il est conseillé de préférer l'option "Patchs automatiques" disponible dans le menu "Système" de l'émulateur plutôt que d'utiliser celle-ci.
("Patchs automatiques" : utilise uniquement le patch associé au jeu émulé) Les jeux suivants nécessitent l'utilisation du hack :
* Bleach : Blade Battlers
* Growlanser II : The Sense of Justice
* Growlanser III : The Dual Darkness
* Wizardry Les jeux suivants nécessitent l'utilisation du hack :
* Shin Mega Tensei : Digital Devil Saga (cinématiques, crashs)
* SSX (bugs graphiques, crashs)
* Resident Evil : Dead Aim (textures) Les jeu suivant nécessite l'utilisation du hack :
* Mana Khemia : Alchemists of Al-Revis (sortir du campus)
 Les jeux suivants nécessitent l'utilisation du hack:
* Test Drive Unlimited
* Transformers La compression NTFS peut être modifiée manuellement depuis l'explorateur Windows (clic droit sur le fichier, puis "Propriétés"). La compression NTFS est sûre et rapide : c'est le format le plus adapté pour les cartes mémoires ! Si le Famelimiting est désactivé, les modes Turbo et Slow Motion ne seront plus disponibles. Attention : le recompiler n'est pas indispensable au fonctionnement de l'émulateur, il permet néanmoins d'améliorer sensisblement les performances globales. Si vous êtes parvenus à résoudre des problèmes en désactivant le recompiler, vous devrez le réactiver manuellement. Attention : L'hardware de la PS2 rend impossible un saut de frames cohérent. L'activation de cette option pourrait entraîner des bugs graphiques conséquents dans certains jeux. Rappel : La plupart des jeux fonctionneront correctement à l'aide du paramétrage par défaut. Rappel : La plupart des jeux fonctionneront correctement à l'aide du paramétrage par défaut. Out of Memory (ou presque) : le SuperVU recompiler n'est pas parvu à allouer suffisamment de mémoire virtuelle, ce qui rend impossible son utilisation. Ceci n'est pas une erreur critique, puisque le SVU recompiler est obsolète et que vous devriez utiliser Microvu ! :p PCSX2 est incapable d'affecter les ressources en mémoire requises pour l'émulation d'une machine virtuelle PS2. Mettez fin aux processus en cours qui nécessitent beaucoup de mémoire (CTRL + ALT + Suppr) puis réessayez. PCSX2 nécessite l'utilisation d'une copie LÉGALE d'un BIOS PS2 pour que l'émulation des jeux fonctionne.
Vous ne pouvez pas avoir recours à une copie obtenue auprès d'un ami ou grâce à internet.
Vous devez faire un dump du BIOS de VOTRE console Playstation 2.  

PCSX2 nécessite un BIOS PS2 valide. D'un point de vue légal, vous DEVEZ obtenir votre BIOS 
à partir de votre propre PS2 (emprunter une console ne satisfait pas ces exigences).
Veuillez consulter la FAQ ou les guides disponibles pour plus de renseignements. L'émulation des jeux Playstation1 n'est pas supportée par PCSX2. Si vous voulez émuler des jeux PSX, veuillez télécharger un émulateur PS1 dédié (par exemple, ePSXe ou PCSX) ! Merci de vous assurer que les dossiers spécifiés sont présents et que votre compte d'utilisateur possède les permissions d'écriture - ou redémarrer PCSX2 en tant qu'administrateur du système, ce qui devrait garantir au programme la capacité à créer ses propres dossiers. Si vous ne disposez pas d'un accès au compte administrateur, vous aurez besoin de faire appel au mode User Documents (cliquez sur le bouton ci-dessous). Veuillez choisir un BIOS valide. Si vous ne possédez pas un tel fichier, cliquez sur Annuler pour fermer le panneau de configuration. Vise surtout l'EE idle loop à l'adresse 0x81FC0 du kernel. Supprime les problèmes de benchmark liés au processus MTGS ou à une surexploitation du GPU. Cette option est plus efficace losqu'elle est utilisée avec une sauvegarde : réalisez votre sauvegarde au moment propice, activez cette option puis chargez à nouveau la partie.

Attention : Cette option peut être activée alors que le jeu a été mis en pause, mais entraînera des bugs graphiques si elle est ensuite désactivée au cours de la même partie. Transfère le VU 1 dans un processus individuel (microVU1 seulement). En général, cela permet une amélioration des performances sur les processeurs 3 coeurs ou plus.
La plupart des jeux supportent bien l'opération, mais certains d'entre eux pourraient tout de même planter.
De plus, les processeurs dual-core souffriront de ralentissements conséquents. Les valeurs définissent la vitesse de l'horlogue du CPU core R5900 de l'EmotionEngine, et entraînent une amélioration des performances conséquente pour les jeux incapables d'utiliser tout le potentiel des composants de la Playstation 2. Les speedhacks améliorent généralement les performances mais peuvent également générer des bugs, empêcher l'émulation du son et fausser la mesure des FPS. Si vous rencontrez des soucis lors de l'émulation d'un jeu, cette option est la première à désactiver. La carte mémoire présente dans le lecteur %d a été automatiquement désactivée. Vous pouvez corriger ce problème
et relancer la carte mémoire à l'aide du menu principal (Configuration -> Cartes mémoire). Les préréglages appliquent des speedhacks, des options spécifiques du recompiler et des patchs particuliers à certains jeux afin d'en améliorer les performances. Toute correction disponible à un problème connu sera automatiquement appliquée.
--> Décochez la case si vous désirez modifier les paramètres manuellement (en utilisant les préréglages actuels comme base) Les préréglages appliquent des speedhacks, des options spécifiques du recompiler et des patchs spécifiques à certains jeux afin d'en améliorer les performances.
Toute correction disponible à un problème connu sera automatiquement appliquée.

Informations :
1 - Le mode d'émulation le plus stable, mais aussi le plus lent.
3 --> Une tentative d'équilibre entre compatibilité et performances.
4 - Utilisation de certains speedhacks.
5 - Utilisation d'un si grand nombre de speedhacks que les performances s'en ressentiront sûrement.
 Le chemin d'accès ou le dossier spécifiés n'existent pas. Voulez-vous le créer ? Le processus '%s' ne répond pas. Il est peut-être bloqué ou s'exécute d'une manière anormalement lente. La mémoire virtuelle disponible est insuffisante, ou l'espace-mémoire a déjà été réservé par d'autres processus, services ou DLL. Une telle opération entraînera la réinitialisation complète de la machine virtuelle PS2 : toutes les données en cours seront perdues.
Êtes-vous sûr de vouloir réaliser cette opération ? Cette opération vise à supprimer les paramètres de %s et à relancer l'assistant de première configuration. 
%s doit être relancé manuellement après que l'opération ait été réalisée avec succès. 

 ATTENTION ! Cliquez sur OK pour supprimer TOUS les paramètres de %s et forcer la fermeture de l'application, provoquant la perte de toutes les données en cours d'exécution.
Êtes-vous sûr de vouloir procéder à cette manipulation ?
PS. Les paramètres des plugins individuels ne seront pas affectés. Ce dossier contient les sauvegardes PCSX2. Vous pouvez enregistrer votre partie en utilisant le menu ou en pressant les touches F1/F3 (sauver/charger). Ce dossier contient les rapports de diagnostique. La plupart des plugins utilisent également le dossier ci-dessous pour enregistrer leurs rapports d'erreurs. Ce dossier contient les screenshots que vous avez pris lors de l'émulation d'un jeu via PCSX2. Le format de l'image et sa qualité dépendent grandement du plugin vidéo GS utilisé. Ce hack fonctionne mieux avec les jeux utilisant le registre INTC Status dans l'attente d'une synchronisation verticale, notamment dans les RPG en 2D. Les jeux qui n'utilisent pas cette méthode connaîtront une légère amélioration des performances. Ce dossier contient les paramètres relatifs à PCSX2, y compris ceux créés par les plugins externes (sauf s'ils sont dépassés techniquement). Les valeurs définissent le nombre de cycles que le Vector Unit "emprunte" au système EmotionEngine. Une valeur élevée augmente le nombre emprunté à l'EE pour chaque microprogramme VU que le jeu utilise. Met à jour les Status Flags uniquement pour les blocs qui les liront, plutôt qu'ils ne soient lus en permanence. Cette option n'occasionne aucun problème en général, et le Super VU fait la même chose par défaut. La synchronisation verticale permet d'éliminer les secousses (tremblements) de l'écran, mais occasionne également des pertes en termes de performances. Il vaut mieux ne l'utiliser qu'en mode plein-écran.

Attention : Certains plugins vidéo ne fonctionnent pas avec cette option ! Attention ! Il est conseillé de réinitialiser complètement l'émulateur PS2 après avoir remplacé un plugin. PCSX2 va maintenant tenter de réaliser une sauvegarde puis de restaurer la session, mais vous pourriez perdre les données en cours si le programme échoue (incompatibilité des plugins).

Êtes-vous sûr de vouloir appliquer ces paramètres ? Attention ! Les lignes de commande se substituent aux paramètres plugins établis.
Ces commandes ne sont pas disponibles dans la fenêtre de configuration des paramètres habituels, 
et seront désactivées si vous cliquez sur le bouton Appliquer. Attention ! Les lignes de commande se substituent aux paramètres établis.
Ces commandes ne sont pas disponibles dans la fenêtre de configuration des paramètres habituels, 
et seront désactivées si vous cliquez sur le bouton Appliquer. Attention ! Certaines fonctions du recompiler PCSX2 n'ont pas pu être initialisées et ont, par conséquent, été désactivées : Attention ! Votre ordinateur ne prend pas en charge les instructions de type SSE2, nécessaires pour la prise en charge d'un grand nombre de plugins et du recompiler PCSX2 : vos options seront limitées et l'émulation (très) lente. Erreur ! Impossible de copier la carte mémoire dans le lecteur %u. Ce dernier est en cours d'utilisation. Vous pouvez modifier le répertoire par défaut du fichier utilisateu PCSX2 (cartes mémoire, paramètres d'affichage, etc...). Cette option prévaut sur tous les autres chemins d'accès spécifiés auparavant. Vous pouvez spécifier un chemin d'accès pour les paramètres PCSX2. Si un fichier de paramétrage existe déjà dans le dossier choisi, vous aurez la possibilité de les importer ou de les écraser. Votre système manque de ressources pour exécuter l'émulateur PCSX2. Cela peut être dû à un autre programme qui occupe trop d'espace-mémoire. Zoom = 100 : les éléments graphiques occupent tout l'écran.
+/- 100 : augmente ou réduit le zoom.
0 : zoom automatique qui supprime les bordures noires.

Attention : certains jeux utilisent volontairement les bordures noires, celles-ci ne seront pas supprimées avec la valeur 0 !

Raccourcis clavier : CTRL + Plus (augmente le zoom) ; CTRL + Moins (diminue le zoom) ; CTRL + * (intevertit les valeurs 100 et 0) 