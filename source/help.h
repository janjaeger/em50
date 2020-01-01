/* Help Text
 *
 *
 * Copyright Notice:
 *
 *   Copyright (C) 1999-2020 Jan Jaeger, All Rights Reserved.
 *
 *
 * This file is part of the Prime 50 Series Emulator (em50).
 *
 *
 * License Statement:
 *
 *   The Prime 50 Series Emulator (em50) is free software:
 *   You can redistribute it and/or modify it under the terms
 *   of the GNU General Public License as published by the
 *   Free Software Foundation, either version 3 of the License,
 *   or (at your option) any later version.
 *
 *   em50 is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *   See the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with em50.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


typedef struct help_t {
  char *h_short;
  char *h_long;
} help_t;


help_t help_assign = { "Assign file to a device",
"Assign [device] [filename] [max_size]\n"
"\n"
"The default filenames for tape are mtdddu and dkdddu for disk.\n"
"\n"
"For example, the default filename associated with unit 0 on device 014 is mt0140,\n"
"and the default filename associated with unit 1 on device 026 is dk0261.\n"
"\n"
"The device must be specified as octal in the format as recognised by the\n"
"BOOT command, or as MT0 .. MT7 for tape devices.\n"
"\n"
"For tapes, an optional maximum tape size can be specified in bytes, K, M or G.\n"
"\n"
"The default location for device files is in the .em50 subdirectory.\n"
"\n"
"For AMLC lines the assign command can be used to connect a line to a line on a\n"
"remote system:\n"
"Assign device [amlc] [line] [host] [port] [remote amlc] [remote line]\n" 
"where amlc is the octal device number, and line is amlc line number (0..15)\n"
"\n"
"For a Primenet Node Controller (PNC) the assign command can be used\n"
"to connect to a remote system:\n"
"Assign device [pnc] [host] [port]\n" 
"where pnc is the octal device number, host is the optional hostname,\n"
"and port is the port PNC is listening on,\n" };

help_t help_boot = { "Boot [options]",
"Boot [l] [sense switches] [data switches] [A register] [B register] [X register] [keys]\n"
"\n"
"sssddd (sense switches in octal)\n"
"100000 Do not execute C_PRMO or PRIMOS.COMI\n"
"040000 Load ring 0 debugger\n"
"020000 Enter debugger after loading PRIMOS\n"
"010000 Do not prompt for device\n"
"004000 Do not prompt for file name\n"
"002000 Halt before executing PRIMOS\n"
"001000 Do not halt on Machine Check\n"
"\n"
"Boot device (ddd, octal)\n"
"\n"
"sssddd (tape)\n"
"000005 Address 014/0 MT0\n"
"000205 Address 014/1 MT1\n"
"000405 Address 014/2 MT2\n"
"000605 Address 014/3 MT3\n"
"000025 Address 013/0 MT4\n"
"000225 Address 013/1 MT5\n"
"000425 Address 013/2 MT6\n"
"000625 Address 013/3 MT7\n"
"\n"
"sssddd (disk)\n"
"000114 Address 026/0\n"
"000314 Address 026/1\n"
"000514 Address 026/2\n"
"000714 Address 026/3\n"
"000134 Address 027/0\n"
"000334 Address 027/1\n"
"000534 Address 027/2\n"
"000734 Address 027/3\n"
"000154 Address 022/0\n"
"000354 Address 022/1\n"
"000554 Address 022/2\n"
"000754 Address 022/3\n"
"000174 Address 023/0\n"
"000374 Address 023/1\n"
"000574 Address 023/2\n"
"000774 Address 023/3\n"
"\n"
"dddddd (data switches in octal - CPBOOT)\n"
"040000 Forces the system to halt after an error\n"
"020000 Inhibits PIO timeouts\n"
"010000 Bypasses diagnostic routines\n"
"004000 Forces error display\n"
"002000 Displays informational messages\n"
"000002 Boots from a pre rev 20 tape\n"
"\n"
"If a legacy boot is required then the first operand must be Legacy,\n"
"this will force legacy interpretation of the boot switches.\n"  };

help_t help_terminal = { "Enter terminal mode", 
"The terminal now emulates the supervisor terminal (asr).\n"
"\n"
"To return to the CP prompt, press the escape key twice.\n"
"\n"
"By default, supervisor terminal (ST) mode will be entered upon a\n"
"START or BOOT command, or <enter> when the CPU is running.\n"
"\n"
"This behaviour can be changed with:\n"
"  TERM CP\n"
"Or back from CP mode to ST mode with:\n"
"  TERM ST\n"
"The current mode can be displayed with:\n"
"  TERM Display" };

help_t help_sysclr = { "Reset / Initialize CPU" };

help_t help_alter = { "Alter register or memory content",
"ALter Virual [address] [value 1] .. [value n]\n"
"  stores the 16 bit word values at the virtual storage location specified by [address].\n"
"\n"
"ALter Real [address] [value 1] .. [value n]\n"
"  stores the 16 bit word values at the real storage location specified by [address].\n"
"\n"
"  Both the address and value(s) are by default in decimal,\n"
"  but can also be specified in octal by means of a leading zero,\n"
"  or in hex by means of a leading zero followed by the character x.\n"
"\n"
"Store is an alias for alter." };

help_t help_run = { "Start CPU",
"Run [address]\n"
"  Enter running mode from either stopped or single step mode.\n"
"  When address (octal) is specifed, processing continues\n"
"  at the specifed address.\n" };

help_t help_stop = { "Stop CPU",
"Enter stopped mode." };

help_t help_step = { "Single Step CPU",
"Execute a single instruction and then stop again." };

help_t help_shell = { "Invoke linux command shell",
"Shell [options]\n"
"\n"
"Invoke the linux command shell, were [options] are the standard\n"
"shell arguments." };

help_t help_quit = { "Terminate EM50",
"Terminates the emulator." } ;

help_t help_serial = { "Set CPU serial number",
"SERial [plantcode] [serial]\n"
"\n"
"  Where plantcode is a 2 position plant code, and the serial number is\n"
"  a 12 digit number.\n"
"  Any serial number can be specified by putting a 14 or 16 character string in quotes." };

#if !defined(MODEL)
help_t help_model = { "Set CPU model and engineering levels",
"MODel [model] [ucodeman] [ucodeeng] [ucodepln] [ucodeext]\n"
"\n"
"  Where ucodeman is the microcode manufactering level,\n"
"  ucodeeng is the microcode engineering level,\n"
"  ucodepln is the processor line, and\n"
"  ucodeext is the microcode entension.\n"
"\n"
"MODel list\n"
"  Will list the supported models." };
#endif

help_t help_sswitch = { "Set Sense Switches",
"SSwitch [value]\n"
"  When a value (octal) is specified, then the contents of the\n"
"  Sense Switches is set to the specifed value, otherwise the\n"
"  value of the Sense Switches is displayed." };

help_t help_sdatasw = { "Set Data Switches",
"SDatasw [value]\n"
"  When a value (octal) is specified, then the contents of the\n"
"  Data Switches is set to the specifed value, otherwise the\n"
"  value of the Data Switches is displayed." };

help_t help_lights = { "Display console lights",
"LIghts\n"
"  Displays the value of the console lights.\n"
"\n"
"LIGHTSC [frequency]\n"
"  Displays the value of the console lights continuously,\n"
"  terminate with <control> C" };

help_t help_display = { "Display register or memory content",
"Display Real [address] [length]\n"
"  Displays the real storage location at the given address.\n"
"\n"
"Display Virtual [address] [length]\n"
"  Displays the virtual storage location at the given address.\n"
"\n"
"  Both the address and length are by default in decimal,\n"
"  but can also be specified in octal by means of a leading zero,\n"
"  or in hex by means of a leading zero followed by the character x\n"
"\n"
"Display Interrupt queue\n"
"  Displays the pending interrupt queue.\n"
"\n"
"Display Lights\n"
"  Displays the value of the console lights.\n"
"\n"
"Display Current register set\n"
"  Displays various fields from the current register set." };

help_t help_input = { "Send command to OS",
"Syntax: input [command text]\n"
"\n"
"  where command text must be in quotes and include a <newline>.\n"
"  e.g. input \"addisk 1234\\n\"\n"
"\n"
"This command is intended for use in the .em50/rc startup file." };

help_t help_sleep = { "Sleep [n] seconds",
"Delay processing of commands for the specifed period.\n"
"\n"
"This command is intended for use in the .em50/rc startup file." };

help_t help_help = { "Provide detailed command information",
"Help [command]" };

#ifdef DEBUG
help_t help_locks = { "Display locks" };

help_t help_trace = { "Activate trace",
"TRACE [on|off] [filename] [close] [append]\n"
"  \"trace off /tmp/tr\" enables the trace\n"
"  \"trace on\" activates instruction level tracing\n"
"  \"trace off\" deactivates instruction level tracing\n"
"  \"trace off close\" closes the trace file\n"
"  Note: the CPU must not be running when close is used." };

help_t help_dump  = { "Force dump to trace file" };
#endif

help_t help_version = { "Version [license]",
"Display version and optional license information." };

help_t help_license = { "License info",
"Display license information." };

help_t help_nohelp = { NULL };
