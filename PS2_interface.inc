;;-----------------------------------------------------------
;;	PS/2 interface functions
;;-----------------------------------------------------------


.equ PS2_DATA_PIN = 6
.equ PS2_CLK_PIN  = 7

	.include "delay.inc"

;;	PS/2 - Utility
;;=====================================================

;PS/2 - Gohi, (go high)
;=====================================================================
	;	Sets the selected PIN in high impedance mode ( DDRDx -> 0 / PORTDx -> 1 )
	;	The Pin is given as a parameter in r16
	;-----------------------------------------------------------------
	GOHI:
		push r16
		
		cpi r16, PS2_DATA_PIN
		brne jmp_clk_gohi
		
		;; On met les pins en entrés avec un pull-up
		cbi DDRD, PS2_DATA_PIN
		sbi PORTD, PS2_DATA_PIN
		rjmp end_gohi
		
		jmp_clk_gohi:
			cbi DDRD, PS2_CLK_PIN
			sbi PORTD, PS2_CLK_PIN

	end_gohi:
		pop r16
	ret

;PS/2 - Golo, (go low)
;=====================================================================
	;	Sets the selected PIN in a low level mode ( DDRDx -> 1 / PORTDx -> 0 )
	;	The Pin is given as a parameter in r16
	;-----------------------------------------------------------------
	GOLO:
		push r16

		cpi r16, PS2_DATA_PIN
		brne jmp_clk_golo

		sbi DDRD, PS2_DATA_PIN
		cbi PORTD, PS2_DATA_PIN
		rjmp end_golo
		
		jmp_clk_golo:
			sbi DDRD, PS2_CLK_PIN
			cbi PORTD, PS2_CLK_PIN

	end_golo:
		pop r16
	ret

;PS/2 - Wait for the low level clock signal
;=====================================================================
	;	Waiting for The device to pull the clock at low
	;-----------------------------------------------------------------
	WAIT_FOR_LOW_PS2:
		loop_fla:
			sbic PIND, PS2_CLK_PIN
			rjmp loop_fla
	ret

;PS/2 - Wait for the high level clock signal
;=====================================================================
	;	Waiting for The device to set the clock at high
	;-----------------------------------------------------------------
	WAIT_FOR_HIGH_PS2:
		loop_wda:
			sbis PIND, PS2_CLK_PIN
			rjmp loop_wda
	ret

;;************************************************************************************************
;PS/2 - Read from the device
;=====================================================================
	;	We return the device message in r18
	;-----------------------------------------------------------------
	
	READ_PS2:		
		push r17
		push r16

		;;We set the clock pin and the data pin as input with a pull-up
		;;in order to initiate the communication
		ldi r16, PS2_DATA_PIN
		rcall gohi

		ldi r16, PS2_CLK_PIN
		rcall gohi

		;; We wait for the null bit which announces the start of the frame
		ldi r16, 3
		rcall DELAY_10mrs

		rcall WAIT_FOR_LOW_PS2
		ldi r16, 1
		rcall DELAY_10mrs
		rcall WAIT_FOR_HIGH_PS2

		;; We parse all the 8 data bits
		ldi r17, 8
		ldi r18, 0

		data_parsing_read:

			rcall WAIT_FOR_LOW_PS2

			;; Si le bit recu est a 1
			LSR r18
			sbic PIND, PS2_DATA_PIN
			subi r18, -0x80

			rcall WAIT_FOR_HIGH_PS2

			dec r17
			brne data_parsing_read

		;; We wait for the parity bit
		rcall WAIT_FOR_LOW_PS2
			;; Here we shoud make some test on the parity bit
		rcall WAIT_FOR_HIGH_PS2
			

		;; Then for the stop bit
		rcall WAIT_FOR_LOW_PS2
		rcall WAIT_FOR_HIGH_PS2
		
		;; And we end the transfert by pulling the clock to low
		ldi r16, PS2_CLK_PIN
		rcall golo

		ldi r16, 1
		rcall DELAY_10mrs

		pop r16
		pop r17
	ret

;PS/2 - Write from the device
;=====================================================================
	;	The data that will be send stands in r19
	;-----------------------------------------------------------------
	WRITE_PS2:
		push r18
		push r17
		push r16
		
		; Inititiating the comunication
		ldi r16, PS2_DATA_PIN
		rcall gohi

		ldi r16, PS2_CLK_PIN
		rcall gohi

		ldi r16, 30
		rcall DELAY_10mrs

		; Request-to-Send
		ldi r16, PS2_CLK_PIN ;;	Set the clock pin at high
		rcall golo

		ldi r16, 30
		rcall DELAY_10mrs
		
		ldi r16, PS2_DATA_PIN ;; Set the data pin at low
		rcall golo

		ldi r16, 1
		rcall DELAY_10mrs

		;;We free up the clock, so it will be managed by our device
		ldi r16, PS2_CLK_PIN
		rcall gohi

		;;We wait for our device to pull the clock at low
		rcall WAIT_FOR_LOW_PS2

		ldi r16 , PS2_DATA_PIN
		ldi r17, 8
		ldi r18, 0; r18 will serve us to compute the parity bit


		data_sending_write:
			;We send the least significant bit of the remaining data
			lsr r19
			brcc data_sfer_write
			rcall gohi
			com r18
			rjmp wait_clock_write
			data_sfer_write: rcall golo

			;; We wait for the raising edge 
			wait_clock_write:
				rcall WAIT_FOR_HIGH_PS2
				rcall WAIT_FOR_LOW_PS2

			dec r17
			brne data_sending_write

		; We send the parity bit
		sbrc r18, 0
		rcall golo	;case: odd

		sbrs r18, 0
		rcall gohi	;case: even

		rcall WAIT_FOR_HIGH_PS2
		rcall WAIT_FOR_LOW_PS2
	

		; The stop bit
		ldi r16, PS2_DATA_PIN
		rcall gohi

		ldi r16, 5
		rcall DELAY_10mrs	; rcall WAIT_FOR_HIGH_PS2
		rcall WAIT_FOR_LOW_PS2

		;ACK
		lp_ack_write:
			sbis PIND, PS2_DATA_PIN		;We wait for the data PIN to be set high
			rjmp lp_ack_write

		rcall WAIT_FOR_HIGH_PS2	;And for the clock PIN

		;We put an end to the transfert
		ldi r16, PS2_CLK_PIN
		rcall golo

		pop r16
		pop r17
		pop r18
	ret
	