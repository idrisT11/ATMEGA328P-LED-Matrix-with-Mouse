

.equ	CS_SPI_PIN	 = 2	
.equ	MOSI_SPI_PIN = 3
.equ	CLK_SPI_PIN	 = 5

;;-----SOWFTWARE IMPLEMENTATION PINs--------
;;	note: Those could be ignored for now
	.equ	SW_CS_SPI_PIN	 = 5
	.equ	SW_MOSI_SPI_PIN  = 6
	.equ	SW_CLK_SPI_PIN	 = 7


	.include "delay.inc"



;Spi - initialisation
;=====================================================================
	;	The value SPCR is given in r16 in parameter
	;-----------------------------------------------------------------
	SPI_INIT:
		sbi DDRB, CS_SPI_PIN
		sbi DDRB, MOSI_SPI_PIN
		sbi DDRB, CLK_SPI_PIN

		sbi PORTB, CS_SPI_PIN

		;ldi r16, 0b01011010		
		out SPCR, r16

	ret

;Spi - Sending a single byte
;=====================================================================
	;	The byte to be send is given in r16
	;-----------------------------------------------------------------
	SPI_SEND:
		
		;We initiate the communication by setting SPDR
		out SPDR, r16

		wait:
			in r16, SPSR
			andi r16, (1 << SPIF)
			breq wait

		in r16, SPDR	; Here we clear the SPIF flag by acceding SPDR

	ret


;Spi - Sending a word
;=====================================================================
	;	The byte to be send is given in r18:r19
	;	Where the most significant byte is stored in r19 
	;	and the least significant is stored in r18 
	;
	;	note: This procedure had been written specificly for max7219 led controler
	;	as we send the most significant byte first
	;-----------------------------------------------------------------
	SPI_SEND_WORD:
		push r16

		cbi PORTB, 2	;We select our device

		mov r16, r19
		rcall SPI_SEND	

		ldi r16, 1
		rcall DELAY_10mrs

		mov r16, r18
		rcall SPI_SEND

		sbi PORTB, 2

		pop r16
	ret

//======================================================================================
;	Here are SPI functions that I have written. As I had some trouble using the SPI interface of the ATmega328p,
;	I wrote those functions as a software interface for the SPI communication specifficly for the MAX7219controler.
;	Even though I am using the microcontroler's SPI interface, I thought it would be intersting if I let those functions here

	; Software SPI - Initialisation
	;=====================================================================
	SW_SPI_INIT:
		sbi DDRD, SW_CS_SPI_PIN
		sbi DDRD, SW_MOSI_SPI_PIN
		sbi DDRD, SW_CLK_SPI_PIN
		
		sbi PORTD, SW_CS_SPI_PIN
	ret	

	; Software SPI - Sending word
	;=====================================================================
	;	The parameter is in r18:r19
	;	r19 being the most significant byte and r18 the least  significant


	upload_word:
		push r16
		push r30

		cbi PORTD, SW_CLK_SPI_PIN	; Clear la clock

		ldi r16, 2
		rcall DELAY_10mrs

		cbi PORTD, SW_CS_SPI_PIN	; Selecting the device by setting CS to "0"

		ldi r30, 16 	; r30 will be used as a counter
	
		data_upload:
			ldi r16, 2
			rcall DELAY_10mrs
			clc
		
			lsl r18
			rol r19
			brcs carry_set	; Carry countains the most significant bit of the remaining data

			cbi PORTD, SW_MOSI_SPI_PIN	
			rjmp update_clock

			carry_set:
			sbi PORTD, SW_MOSI_SPI_PIN	

			update_clock:
			ldi r16, 2
			rcall DELAY_10mrs

			sbi PORTD, SW_CLK_SPI_PIN
			cbi PORTD, SW_CLK_SPI_PIN	
			
			dec r30
			brne data_upload
				;We branch as long as the counter differs from 0
	

		ldi r16, 2
		rcall DELAY_10mrs

		sbi PORTD, SW_CS_SPI_PIN	; We deselecting our device
		
		pop r30
		pop r16
	ret

;;------------------------------------------------------------------------------
;;	Ifouk lKaghet
;;------------------------------------------------------------------------------

