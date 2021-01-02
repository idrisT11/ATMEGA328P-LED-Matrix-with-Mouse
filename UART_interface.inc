;;------------------------------------------------------------
;;	Uart interface
;;------------------------------------------------------------

.ifndef UART_DEFINITION
.equ UART_DEFINITION = 0

;UART - Initialisation
;=====================================================================
	UART_INIT:
		ldi r16, 103
		sts UBRR0L, r16
		ldi r16, 0
		sts UBRR0H, r16

		ldi r16, 0x18	
		sts UCSR0B,r16		;UCSR0B = CONTROL		
	ret

;UART - Receiving
;=====================================================================
	;;	The received byte will be stocked in r16
	;;----------------------------------------------------------------
	rx_uart:
		lds r16, UCSR0A		;UCSR0A = STATE
		andi r16, 0x80
		breq rx_uart

		lds r16, UDR0
	ret


;UART - Transmitting
;=====================================================================
	;;	The byte to be send is given in r16 as a parameter
	;;----------------------------------------------------------------
	tx_uart:
		push r17

		transmition_uart:
			lds r17, UCSR0A
			andi r17, 0x20	; UDRE0
			breq transmition_uart

		sts UDR0, r16

		pop r17
	ret

;UART - Transmitting in readable Hexadecimal
;=====================================================================
	;;	The byte to be send is given in r16 as a parameter
	;;----------------------------------------------------------------

	send_byte:
		push r16
		push r17
		push r18
		push r19
				
		mov r18, r16
		mov r19, r16

		;Sending the '0x' part
		ldi r16, '0'
		rcall tx_uart
		ldi r16, 'x'
		rcall tx_uart

		;The most significant 4 bits first
		andi r19, 0xf0
		lsr r19
		lsr r19
		lsr r19
		lsr r19 ; Hmmmmmm
		mov r16, r19
		rcall ascii_dec_hex
		rcall tx_uart

		;Then the least significant
		andi r18, 0x0f
		mov r16, r18
		rcall ascii_dec_hex
		rcall tx_uart

		;The final space
		ldi r16, 0x20
		rcall tx_uart


		pop r19
		pop r18
		pop r17
		pop r16
	ret

	;A utility function that convert a 4 bits number into its Acsii form
	;;----------------------------------------------------------------
	ascii_dec_hex:
		cpi r16, 0xA
		brge kbir_f_A_send_byte
		subi r16, -48
		ret
		kbir_f_A_send_byte:
		subi r16, -55
	ret

.endif
