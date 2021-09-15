#pragma once

namespace Ascii
{
	enum class ControlChar
	{
		/* copied from https://www.w3schools.com/charsets/ref_html_ascii.asp */
		NUL	= 0,	// null character
		SOH	= 1,	// start of header
		STX	= 2,	// start of text
		ETX	= 3,	// end of text
		EOT	= 4,	// end of transmission
		ENQ	= 5,	// enquiry
		ACK	= 6,	// acknowledge
		BEL	= 7,	// bell (ring)
		BS	= 8,	// backspace
		HT	= 9,	// horizontal tab
		LF	= 10,	// line feed
		VT	= 11,	// vertical tab
		FF	= 12,	// form feed
		CR	= 13,	// carriage return
		SO	= 14,	// shift out
		SI	= 15,	// shift in
		DLE	= 16,	// data link escape
		DC1	= 17,	// device control 1
		DC2	= 18,	// device control 2
		DC3	= 19,	// device control 3
		DC4	= 20,	// device control 4
		NAK	= 21,	// negative acknowledge
		SYN	= 22,	// synchronize
		ETB	= 23,	// end transmission block
		CAN	= 24,	// cancel
		EM	= 25,	// end of medium
		SUB	= 26,	// substitute
		ESC	= 27,	// escape
		FS	= 28,	// file separator
		GS	= 29,	// group separator
		RS	= 30,	// record separator
		US	= 31,	// unit separator
		DEL	= 127,	// delete (rubout)
	};

	bool isAlphaNumeric(char c);
	bool isChar(char c);
	bool isControlChar(char c);
	bool checkControlChar(char c, ControlChar controlChar);
};
