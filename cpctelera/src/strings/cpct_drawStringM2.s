;;-----------------------------LICENSE NOTICE------------------------------------
;;  This file is part of CPCtelera: An Amstrad CPC Game Engine 
;;  Copyright (C) 2014-2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
;;
;;  This program is free software: you can redistribute it and/or modify
;;  it under the terms of the GNU General Public License as published by
;;  the Free Software Foundation, either version 3 of the License, or
;;  (at your option) any later version.
;;
;;  This program is distributed in the hope that it will be useful,
;;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;;  GNU General Public License for more details.
;;
;;  You should have received a copy of the GNU General Public License
;;  along with this program.  If not, see <http://www.gnu.org/licenses/>.
;;-------------------------------------------------------------------------------
.module cpct_strings

;;
;; Include constants and general values
;;
.include /strings.s/
.globl _cpct_drawCharM2_asm

;
;########################################################################
;## FUNCTION: _cpct_drawROMStringM2                                   ###
;########################################################################
;### This function receives a null terminated string and draws it to  ###
;### the screen in mode 2, using _cpct_drawROMCharM2 to draw every    ###
;### character.                                                       ###
;### * Some IMPORTANT things to take into account:                    ###
;###  -- This routine does not check for boundaries. If you draw too  ###
;###     long strings or out of the screen, unpredictable resuls will ###
;###     happen.
;###  -- Do not put this function's code below 4000h in memory. In    ###
;###     order to read from ROM, this function enables Lower ROM      ###
;###     (which is located 0000h-3FFFh), so CPU would read from ROM   ###
;###     instead of RAM in first bank, effectively shadowing this     ###
;###     piece of code, and producing undefined results (tipically,   ###
;###     program would hang or crash).                                ###
;###  -- This function works well for drawing on double buffers loca- ###
;###     ted at whichever memory bank, except 0000h (4000h-FFFFh)     ###
;###  -- This function disables interrupts during main loop (charac-  ###
;###     ter printing). It reenables them at the end.                 ###
;###  -- Do not pass numbers greater that 15 as color parameters, as  ###
;###     they are used as indexes in a color table, and results may   ###
;###     be unpredictable                                             ###
;########################################################################
;### INPUTS (5 Bytes)                                                 ###
;###  * (2B HL) Pointer to the null terminated string being drawn     ### 
;###  * (2B DE) Video memory location where the char will be printed  ### 
;###  * (1B B) Foreground color (PEN, 0-1)                            ###
;########################################################################
;### EXIT STATUS                                                      ###
;###  Destroyed Register values: AF, BC, DE, HL                       ###
;########################################################################
;### MEASURES                                                         ###
;### MEMORY:  41 bytes (+ 144 bytes drawROMCharM0)                    ###
;### TIME:                                                            ###
;###  Best case  = 179 + (120+3704)*len cycles                        ###
;###  Worst case = 179 + (120+4384)*len cycles                        ###
;########################################################################
;

_cpct_drawStringM2::
   ;; GET Parameters from the stack (Pop + Restoring SP)
   ld (drsm2_restoreSP+1), sp          ;; [20] Save SP into placeholder of the instruction LD SP, 0, to quickly restore it later.
   di                                  ;; [ 4] Disable interrupts to ensure no one overwrites return address in the stack
   pop  af                             ;; [10] AF = Return Address
   pop  hl                             ;; [10] HL = Pointer to the null terminated string
   pop  de                             ;; [10] DE = Destination address (Video memory location where character will be printed)
   pop  bc                             ;; [10] C = Foreground color (0 / 1, Background inverted) 
drsm2_restoreSP:
   ld sp, #0                           ;; [10] -- Restore Stack Pointer -- (0 is a placeholder which is filled up with actual SP value previously)
   ei                                  ;; [ 4] Enable interrupts again

   ld a, c                             ;; [ 4] A = Foreground color
   ld (drsm2_firstChar+1), a           ;; [ 7] Save foreground color in its placeholder to be restored at every step in the loop
   jp drsm2_firstChar                  ;; [10] Jump to first char

drsm2_nextChar:
   push hl                             ;; [11] Save HL and DE to the stack befor calling draw char
   push de                             ;; [11]
   ld  b, a                            ;; [ 4] B = Next character to be drawn
   call _cpct_drawCharM2_asm           ;; [17] Draw next char
   pop  de                             ;; [10] Recover HL and DE from the stack
   pop  hl                             ;; [10]

   ;; Increment pointer values
   inc  de                             ;; [ 6] DE += 1 (point to next position in video memory, 8 pixels to the right)
   inc  hl                             ;; [ 6] HL += 1 (point to next character in the string)

drsm2_firstChar:
   ld  c, #0                           ;; [ 7] Restore C value (Foreground color)
   ld  a, (hl)                         ;; [ 7] A = next character from the string
   or  a                               ;; [ 4] Check if A = 0
   jp  nz, drsm2_nextChar              ;; [10] if A != 0, A is next character, draw it, else end

drsm2_endString:
   ret                                 ;; [10] Return