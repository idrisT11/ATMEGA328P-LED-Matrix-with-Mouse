;;-----------------------------------------------------------
;;	User interface functions
;;-----------------------------------------------------------
;;	Functions here manage the onscreen cursor, the user interaction
;;	and the screen's buffer
;;-----------------------------------------------------------

	.include "UART_interface.inc"
	.include "Mouse_interface.inc"

.equ SCREEN_BUFFER_SIZE=16

.dseg
	.org SRAM_START + END_MOUSE_BUFFER

	cursor: .BYTE 1	; the MS 4bits encode the Y position and the LS ones encode the X position( X/Y is set in the [1:8] interval )
	isCursorOn:	.Byte 1	;if this byte is at 0x01, we print the cursor on screen, if at 0x00, it is not printed
	screen_line: .BYTE 2	; this encode the part of the screen buffer that will be printed
	video_buffer: .BYTE SCREEN_BUFFER_SIZE

.cseg

;UI - Initiate the UI interface 
;=====================================================================
	UI_INIT:
		push r17
		push r16

		;; setting the cursor blinking timer
		ldi r16, low(-4687)
		;out TCNT1L, r16
		ldi r16, high(-4687)
		;out TCNT1H, r16

		ldi r16, 0b00000101
		;out TCCR1B, r16

		;ldi r16, 0b10000000
		;out TIMSK, r16
		
		ldi r16, 0x55
		sts cursor, r16

		ldi r16, 0x01
		sts isCursorOn, r16

		;; initiating the video buffer
		ldi r16, 0
		sts screen_line, r16	;We set the top of the screen to the top of the screen buffer

		ldi ZL, low(video_buffer)
		ldi ZH, high(video_buffer)

		ldi r17, SCREEN_BUFFER_SIZE;We clear the video buffer 
		clear_buffer_ui:
			st Z+, r16
			dec r17
			brne clear_buffer_ui

	
		rcall RENDER_UI

		pop r16
		pop r17
		 
	ret

;UI - Update the cursor position
;=====================================================================
	;	Here we update the cursor position if needed
	;-----------------------------------------------------------------
	HANDLE_EVENT_UI:
		push r18
		push r17
		push r16
		
		;;CURSOR HANDLING
		;;-----------------------------------------------------------------

		ldi r16, 0	; X component
		rcall UPDATE_CURSOR_COMPOSENT_UI

		ldi r16, 1	; Y component
		rcall UPDATE_CURSOR_COMPOSENT_UI
		

		;;CLICK HANDLING
		;;-----------------------------------------------------------------
		lds r16, state_packet
		andi r16, 0x03
		cpi r16, 0
		breq wheel_handling_ui

		lds r16, screen_line
		ldi r17, 0
		ldi ZL, low(video_buffer)
		ldi ZH, high(video_buffer)

		add ZL, r16
		adc ZH, r17

		lds r16, cursor
		mov r18, r16
		andi r18, 0x0f
		andi r16, 0xf0
		swap r16

		
		add ZL, r16
		adc ZH, r17
		sbiw Z, 1

		ldi r17, 0x00
		sec
		lp_click_event_ui:	
			rol r17			
			dec r18
			brne lp_click_event_ui

		ld r18, Z

		lds r16, state_packet
		sbrs r16, 0
		rjmp right_click_handle_ui
		
		or r17, r18				;If the user made a left click
		rjmp update_after_click_ui

		right_click_handle_ui:	;If the user made a right click
		com r17
		and r17, r18
		mov r16, r17
		rcall send_byte

		update_after_click_ui:	st Z, r17

		
		;;WHEEL HANDLING
		;;-----------------------------------------------------------------
		wheel_handling_ui:

		lds r16, Z_packet
		
		pop r16
		pop r17
		pop r18
	ret
	
	;UI - Update the cursor's component position
	;-----------------------------------------------------------------
	;	This function update the cursor X or Y component
	;	It takes as a parameter in r16, the component on which we want to work on
	;	as for the X component mode we set: r16=0 and the Y mode: r16=1
	;-----------------------------------------------------------------
	UPDATE_CURSOR_COMPOSENT_UI:
		
		push r20
		push r19
		push r18
		push r17
		
		;We load the right packet
		lds r17, cursor		
		lds r19, cursor		
		cpi r16, 0
		breq load_X_packet_update_ui

	;loading the Y packet
		lds r18, Y_packet
		andi r19, 0xf0		; r19 contains the cursor's composent we are currently working on (X or Y)
		andi r17, 0x0f		; r17 will contain the other cursor's composent (X or Y)
		swap r19	;we swap the 4MSbits and the 4LSbits of r19
		rjmp traitement_update_ui
	
	;loading the X packet
	load_X_packet_update_ui:
		lds r18, X_packet 
		andi r17, 0xf0
		andi r19, 0x0f
		

		traitement_update_ui:
			cpi r18, 0
			breq end_update_component_ui	; If the packet's value is null

			lds r20, state_packet

			sbrs r16, 0		;We extract the Negative flag 
			andi r20, 0x10
			sbrc r16, 0
			andi r20, 0x20
			

			cpi r20, 0
			breq divide_update_ui
			neg r18		; If the negative flag is set, we complement our register
			inc r18
			divide_update_ui:
			lsr r18
			lsr r18
			lsr r18
			breq end_update_component_ui	;If the mouvement is too small

			cpi r20, 0						;here we set new cursor composent's value
			breq negative_changement_ui	
			subi r19, -1	; the direction is different for each of the X and Y components
			rjmp set_in_range_ui
			negative_changement_ui:	
				subi r19, 1

			set_in_range_ui:	;Here we verify if the new cursor value is set in the 1-8 range, otherwise we affect it with either 1 or 8
				cpi r19, 9
				brlo smaller_update_ui
				ldi r19, 8	; if the cursor component is above than 8
				rjmp update_cursor_buffer_ui
				smaller_update_ui:
				cpi r19, 0	; if the cursor component is smaller than 1 (ia, = 0)
				brne update_cursor_buffer_ui
				ldi r19, 1

			update_cursor_buffer_ui:
				sbrc r16, 0
				swap r19		;we reswap r19
				or r17, r19
				sts cursor, r17

		end_update_component_ui:

		pop r17
		pop r18
		pop r19
		pop r20
	ret


;UI - Rendering on the Matrix
;=====================================================================
	;-----------------------------------------------------------------
	RENDER_UI:
		push r18
		push r17
		push r16

		ldi ZL, low(video_buffer)
		ldi ZH, high(video_buffer)

		lds r16, screen_line
		ldi r17, 0

		clc
		add ZL, r16
		adc ZH, r17

		ldi r16, 0x01

		print_buffer_ui:
			ld r17, Z+
			rcall SET_ROW_MATRIX	; r16 -> the row number	/	r17 -> the row state
			inc r16
			cpi r16, 9
			brne print_buffer_ui

		lds r19, isCursorOn
		rcall WRITE_CURSOR_SCREEN_UI
			

		pop r16
		pop r17
		pop r18
	ret

;UI - Handling the cursor blinking
	HANDLE_CURSOR_UI:

	reti

;UI - Printing the cursor into the matrix
;=====================================================================
	;	This function takes as a parameter in r19 the state of the cursor
	;	as for r19=0, the cursor will be turned off
	;	and for r19=1, it will be turn on
	;-----------------------------------------------------------------
	WRITE_CURSOR_SCREEN_UI:
		push r17
		push r16

		ldi ZL, low(video_buffer)
		ldi ZH, high(video_buffer)

		lds r16, screen_line
		ldi r17, 0
		add ZL, r16
		adc ZH, r17

		lds r16, cursor
		mov r18, r16
		andi r16, 0xf0
		andi r18, 0x0f
		swap r16

		add ZL, r16
		adc ZH, r17
		sbiw ZL, 1	;	Because r16 is found in [1:8]

		ldi r17, 0x00
		sec
		lp_set_cursor_event_ui:	
			rol r17			
			dec r18
			brne lp_set_cursor_event_ui
		
		ld r18, Z

		cpi r19, 0
		breq turn_off_cursor_ui
		or r17, r18
		rjmp print_cursor_matrix_ui

		turn_off_cursor_ui:
		mov r17, r18
		print_cursor_matrix_ui:	rcall SET_ROW_MATRIX

		pop r16
		pop r17
	ret
		

		
			

