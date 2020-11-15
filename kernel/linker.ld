ENTRY(_start)
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
	. = 0x00100000;
	.text :
	{
		_text = .;
		*(.text);
		_text_end = .;
	}
	
	.data :
	{
		_data = .;
		*(.bss);
		*(.bss*);
		*(.data);
		*(.rodata*);
		*(COMMON)
		_data_end = .;
	}
	
	/DISCARD/ :
	{
		*(.note*);
		*(.iplt*);
		*(.igot*);
		*(.rel*);
		*(.comment);
	}
}