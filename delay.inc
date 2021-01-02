;;------------------------------------------------------------
;;	Delay functions
;;
;;	Important: I work here on a Atmega328p clocked at 16Mhz
;;------------------------------------------------------------

.ifndef DELAY_DEFINITION
.equ DELAY_DEFINITION = 0

;Delay - In milliseconds
;=====================================================================

	DELAY_mls:
	;	The parameter is given in r22:r23 in milliseconds
	;----------------------------------------------------------
		push r24
		push r25

		lp_delay_old:

			ldi r24, low(4000)
			ldi r25, high(4000)
		
			lpi_delay_old:
				sbiw r24, 1			; sbiw: 2 clock cycles
				brne lpi_delay_old	; brne: 2 clock cycles if a jump was made, and 1 if not

			subi r16, 1
			sbci r17, 0
			brne lp_delay_old

		pop r25
		pop r24
	ret

;Delay - In microseconds
;=====================================================================

	DELAY_10mrs:
	;	The parameter is given in r16 as 10 * 1 microsecond
	;	So this function can only generate a delay of a multiple of 10 microsecond
	;----------------------------------------------------------
		push r17
		
		lp_delay:
			ldi r17, 53
			lpi_delay:
				dec r17
				brne lpi_delay
			dec r16
			breq lp_delay
		pop r17
	ret


.endif

