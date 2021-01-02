;;------------------------------------------------------------
;;	Matrix interface functions
;;------------------------------------------------------------
	
	.include "SPI_interface.inc"
	.include "UART_interface.inc"
	.include "delay.inc"

;Matrix - Initialisation
;=====================================================================
	MATRIX_INIT:
		push r19
		push r18
		push r16


		ldi r16, 0b01011010
		rcall SPI_INIT		; We initiate the SPI interface

		;Set to normal: off
		ldi r18, 0x00
		ldi r19, 0x0C
		rcall SPI_SEND_WORD

		;Test mode : off
		ldi r18, 0x00
		ldi r19, 0x0F
		rcall SPI_SEND_WORD

		;Decode mode: NO CHARACTER DECODING
		ldi r18, 0x00
		ldi r19, 0x09
		rcall SPI_SEND_WORD

		;Brightness
		ldi r18, 0x08
		ldi r19, 0x0A
		rcall SPI_SEND_WORD

		;scan_limit
		ldi r18, 0x07
		ldi r19, 0x0B
		rcall SPI_SEND_WORD

		;Test mode : on
		ldi r18, 0x01
		ldi r19, 0x0C
		rcall SPI_SEND_WORD

		pop r16
		pop r18
		pop r19
	ret

;Matrix - Clear
;=====================================================================
	CLEAR_MATRIX:
		push r19
		push r18

		ldi r18, 0x00
		ldi r19, 0x08
		lp_clear_matrix:
			rcall SPI_SEND_WORD	
			dec r19
			brne lp_clear_matrix

		pop r18
		pop r19
	ret


;Matrix - SET ROW
;=====================================================================
	;	The row number is given in r16 
	;	And its state in r17
	;-----------------------------------------------------------------
	SET_ROW_MATRIX:
		push r19
		push r18

		mov r18, r17		;I could have change the parameter to r18:r19, but I prefered having here r16:r17
		mov r19, r16
		rcall SPI_SEND_WORD	
		

		pop r18
		pop r19
	ret



	

