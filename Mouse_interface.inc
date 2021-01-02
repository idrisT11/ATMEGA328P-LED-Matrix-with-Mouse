;;-----------------------------------------------------------
;;	Mouse interface functions
;;-----------------------------------------------------------
;;	Note: I don't know why, but for some reason the mouse that I am using for my test
;;	seems to respond in frame of two bytes instead of one, (exemple : 0xf4-0xfa instead of just 0xfa)
;;	I did not found anything like that on internet, so I am not able to say if the problem come from 
;;	the PS/2 interface implementation or from the mouse.

.ifndef MOUSE_DEFINITION
.equ MOUSE_DEFINITION = 0

.equ MOUSE_BUFFER = SRAM_START

.dseg
	.org MOUSE_BUFFER

	state_packet: .BYTE 1
	X_packet: .BYTE 1
	Y_packet: .BYTE 1
	Z_packet: .BYTE 1

	END_MOUSE_BUFFER: .BYTE 1
.cseg


	.include "PS2_interface.inc"
	.include "UART_interface.inc"

;MOUSE - Initialisation
;=====================================================================
	;;	The data that will be send stands in r16
	;;----------------------------------------------------------------
	MOUSE_INIT:
		push r16

		ldi r16, PS2_DATA_PIN
		rcall GOHI

		ldi r16, PS2_CLK_PIN
		rcall GOHI

		;RESET
		ldi r19, 0xff
		rcall WRITE_PS2	; reset commande
		
		rcall READ_PS2	; acknowledgement (??)0xf4
		rcall READ_PS2	; 0xfa	
		rcall READ_PS2	; 0xaa
		rcall READ_PS2	; 0x00
		

		;ENABLE DATA REPORTING
		ldi r19, 0xf4
		rcall WRITE_PS2	; enable data reporting

		rcall READ_PS2	
		rcall READ_PS2	

		;DOES IT HAVE A WHEEL ?
		rcall WHEEL_TEST_INIT	; Activating the microsoft intellimouse mode
		
		;Set the sample rate to 60samples/s
		ldi r19, 0xf3
		rcall WRITE_PS2	

		rcall READ_PS2
		rcall READ_PS2	

		ldi r19, 60
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		;Set the resolution to 1count/mm
		ldi r19, 0xE8
		rcall WRITE_PS2	

		rcall READ_PS2
		rcall READ_PS2	

		ldi r19, 0x03
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		ldi r16, 5
		rcall DELAY_10mrs

		pop r16
	ret

;MOUSE - Activating the intellimouse mode and verifying the mouse type
;=====================================================================
	;;	The result will be set in r16
	;;	0x00 -> There is no wheel	/	0x03 -> There is a wheel
	;;	To activated the intellimouse mode we have to follow  a list of specific commandes to the device
	;;	Which are : 
	;;		Scale 200 -> Scale 100 -> Scale 80 -> GET ID
	;;----------------------------------------------------------------
	WHEEL_TEST_INIT:
		
		push r19
		
		;;SCALE 200
		ldi r19, 0xf3
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		ldi r19, 200
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		;;SCALE 100
		ldi r19, 0xf3
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		ldi r19, 100
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		;;SCALE 80
		ldi r19, 0xf3
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		ldi r19, 80
		rcall WRITE_PS2	

		rcall READ_PS2	
		rcall READ_PS2	

		;;GET ID
		ldi r19, 0xf2
		rcall WRITE_PS2	

		rcall READ_PS2
		rcall READ_PS2
		rcall READ_PS2

		mov r16, r18

		pop r19
	ret

;MOUSE - Poll for a frame
;=====================================================================
	;;	When the procedure detects a new frame, it will
	;;	save its information in the "mouse buffer"
	;;----------------------------------------------------------------
	READ_MOUSE:
		push r18

		rcall READ_PS2
		sts state_packet, r18

		rcall READ_PS2
		sts X_packet, r18

		rcall READ_PS2
		sts Y_packet, r18

		rcall READ_PS2
		sts Z_packet, r18

		pop r18
	ret

.endif
