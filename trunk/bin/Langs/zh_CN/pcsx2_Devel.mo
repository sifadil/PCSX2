Þ    "      ,  /   <      ø  3   ù  >   -  5   l  7   ¢  Z   Ú  J   5  (     "   ©  >   Ì  ?     ;   K  A     6   É  2      2   3  6   f  *     .   È  #   ÷       G   8  U         Ö  @   ÷  D   8     }          ³  )   Ð  6   ú  7   1	  A   i	  B   «	  ì  î	  2   Û  H     G   W  ;     R   Û  B   .  %   q  !     <   ¹  <   ö  8   3  B   l  +   ¯  +   Û  '     3   /  .   c  .        Á     Ý  @   ÷  H   8        <   ¢  B   ß     "     >     X  .   u  9   ¤  :   Þ  F     D   `                                                                                                	      !                       "                             
    Actual DMA event processing and data transfer logs. Actual data transfer logs, bus right arbitration, stalls, etc. All GIFtag parse activity; path index, tag type, etc. All VIFcode processing; command, tag style, interrupts. All known hardware register accesses (very slow!); not including sub filter options below. All known hardware register accesses, not including the sub-filters below. All processing involved in Path3 Masking Detailed logging of CDVD hardware. Direct memory accesses to unknown or unmapped EE memory space. Direct memory accesses to unknown or unmapped IOP memory space. Disasm of COP0 instructions (MMU, cpu and dma status, etc). Disasm of executing core instructions (excluding COPs and CACHE). Disasm of the EE's VU0macro co-processor instructions. Disasm of the EE's floating point unit (FPU) only. Disasm of the IOP's GPU co-processor instructions. Dumps detailed information for PS2 executables (ELFs). Dumps various GIF and GIFtag parsing data. Dumps various VIF and VIFcode processing data. Execution of EE cache instructions. Gamepad activity on the SIO. IPU activity: hardware registers, decoding operations, DMA status, etc. Logs manual protection, split blocks, and other things that might impact performance. Logs only DMA-related registers. Logs only unknown, unmapped, or unimplemented register accesses. Memorycard reads, writes, erases, terminators, and other processing. SYSCALL and DECI2 activity. SYSCALL and IRX activity. Scratchpad's MFIFO activity. Shows DECI2 debugging logs (EE processor) Shows the game developer's logging text (EE processor) Shows the game developer's logging text (IOP processor) Tracks all EE counters events and some counter register activity. Tracks all IOP counters events and some counter register activity. Project-Id-Version: PCSX2 0.9.7
Report-Msgid-Bugs-To: http://code.google.com/p/pcsx2/
POT-Creation-Date: 2011-03-14 18:10+0100
PO-Revision-Date: 2011-02-26 15:42+0800
Last-Translator: Wei Mingzhi <whistler_wmz@users.sf.net>
Language-Team: 
Language: 
MIME-Version: 1.0
Content-Type: text/plain; charset=UTF-8
Content-Transfer-Encoding: 8bit
X-Poedit-KeywordsList: pxE_dev;pxDt
X-Poedit-SourceCharset: utf-8
X-Poedit-Basepath: trunk\
X-Poedit-SearchPath-0: pcsx2
X-Poedit-SearchPath-1: common
 å®éç DMA äºä»¶å¤çåæ°æ®ä¼ è¾æ¥å¿ã å®éçæ°æ®ä¼ è¾æ¥å¿ãæ»çº¿æéä»²è£ãæ»çº¿é»å¡ç­ç­ã å¨é¨ GIF æ ç­¾è§£ææ´»å¨ï¼è·¯å¾ç´¢å¼ãæ ç­¾ç±»åï¼ç­ç­ã å¨é¨ VIF ä»£ç å¤çï¼å½ä»¤ãæ ç­¾é£æ ¼ãä¸­æ­ã å¨é¨å·²ç¥ç¡¬ä»¶å¯å­å¨è®¿é® (å¾æ¢!)ï¼ä¸åæ¬ä»¥ä¸å­è¿æ»¤å¨éé¡¹ã å¨é¨å·²ç¥ç¡¬ä»¶å¯å­å¨è®¿é®ï¼ä¸åæ¬ä»¥ä¸å­è¿æ»¤å¨ã å¨é¨ä¸ Path3 æ å¿æå³çå¤ç è¯¦ç»è®°å½ CDVD ç¡¬ä»¶ä¿¡æ¯ã å°æªç¥ææªæ å°çåå­ç©ºé´çç´æ¥åå­è®¿é®ã å°æªç¥ææªæ å°çåå­ç©ºé´çç´æ¥åå­è®¿é®ã åæ±ç¼ COP0 æä»¤ (MMUï¼CPU/DMA ç¶æï¼ç­ç­)ã åæ±ç¼æ§è¡æ ¸å¿æä»¤ (é¤äº COP æä»¤å CACHE æä»¤)ã åæ±ç¼ EE VU0macro åå¤çå¨æä»¤ã ä»åæ±ç¼ EE æµ®ç¹è¿ç®åå (FPU)ã åæ±ç¼ IOP GPU åå¤çå¨æä»¤ã è½¬å¨ PS2 å¯æ§è¡æä»¶ (ELF) çè¯¦ç»ä¿¡æ¯ã è½¬å¨åç§ GIF å GIF æ ç­¾è§£ææ°æ®ã è½¬å¨åç§ VIF å VIF ä»£ç å¤çæ°æ®ã EE ç¼å­æä»¤çæ§è¡ã SIO ä¸çæææ´»å¨ã IPU æ´»å¨: ç¡¬ä»¶å¯å­å¨ãè§£ç æä½ãDMA ç¶æç­ç­ã è®°å½æå¨ä¿æ¤ãåå²åä»¥åå¶å®å¯è½å½±åæ§è½çä¸è¥¿ã ä»è®°å½ DMA ç¸å³å¯å­å¨ã ä»è®°å½æªç¥ãæªæ å°ææªå®ç°çå¯å­å¨è®¿é®ã è®°å¿å¡è¯»åãåå¥ãæ¦é¤ãç»æ­¢ç¬¦ï¼åå¶å®æä½ã SYSCALL å DECI2 æ´»å¨ã SYSCALL å IRX æ´»å¨ã æå­å¨ç MFIFO æ´»å¨ã æ¾ç¤º DECI2 è°è¯æ¥å¿è®°å½ (EE å¤çå¨) æ¾ç¤ºæ¸¸æå¼åèçæ¥å¿è®°å½ææ¬ (EE å¤çå¨) æ¾ç¤ºæ¸¸æå¼åèçæ¥å¿è®°å½ææ¬ (IOP å¤çå¨) è·è¸ªææç EE è®¡æ°å¨äºä»¶åä¸äºè®¡æ°å¨å¯å­å¨æ´»å¨ã è·è¸ªææ IOP è®¡æ°å¨äºä»¶åä¸äºè®¡æ°å¨å¯å­å¨æ´»å¨ã 