#as: -march=rv64i_zbkb
#source: zbkb-64.s
#objdump: -d

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <target>:
[ 	]+.*:[ 	]+.*[ 	]+ror[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+rol[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+rori[ 	]+a0,a1,0x2
[ 	]+.*:[ 	]+.*[ 	]+rorw[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+rolw[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+roriw[ 	]+a0,a1,0x2
[ 	]+.*:[ 	]+.*[ 	]+andn[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+orn[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+xnor[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+pack[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+packh[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+packw[ 	]+a0,a1,a2
[ 	]+.*:[ 	]+.*[ 	]+brev8[ 	]+a0,a0
[ 	]+.*:[ 	]+.*[ 	]+rev8[ 	]+a0,a0
