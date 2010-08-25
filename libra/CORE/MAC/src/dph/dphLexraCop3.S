/*
 * Airgo Networks, Inc proprietary. All rights reserved.
 *
 * Author:      Pierre Vandewalle
 * Date:        09/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 * 
 */
	

#define zero    $0
#define AT      $1
#define v0      $2
#define v1      $3
#define a0      $4
#define a1      $5
#define a2      $6
#define a3      $7
#define t0      $8 
#define t1      $9
#define t2      $10
#define t3      $11
#define t4      $12
#define t5      $13
#define t6      $14
#define t7      $15
#define s0      $16
#define s1      $17
#define s2      $18
#define s3      $19
#define s4      $20
#define s5      $21
#define s6      $22
#define s7      $23
#define t8      $24
#define t9      $25
#define jp      $25 
#define k0      $26 
#define k1      $27
#define gp      $28 
#define sp      $29
#define fp      $30
#define s8		$30
#define ra      $31 
	

	
.set noreorder		
.align	4
.globl enable_cp3		
enable_cp3:
mfc0 t1,$12
nop
lui t0,0x8000
or t1,t1,t0
mtc0 t1,$12
nop
nop
nop
ctc3 zero, $0
nop
nop
mtc3 zero,$8		
mtc3 zero,$9	
mtc3 zero,$10		
mtc3 zero,$11	
mtc3 zero,$12	
mtc3 zero,$13	
mtc3 zero,$14
mtc3 zero,$15	
nop
nop
jr ra
nop

.align	4
.globl clear_cp3		
clear_cp3:
nop
ctc3 zero, $0
nop
nop
mtc3 zero,$8		
mtc3 zero,$9	
mtc3 zero,$10		
mtc3 zero,$11	
mtc3 zero,$12	
mtc3 zero,$13	
mtc3 zero,$14
mtc3 zero,$15	
nop
nop
jr ra
nop

.align	4
.globl start_cp3		
start_cp3:
ctc3 a0, $0
jr ra
nop

.align	4
.globl stop_cp3		
stop_cp3:
ctc3 zero, $0
jr ra
nop

.align	4
.globl get_cp3_0	
get_cp3_0:
mfc3 v0, $8
jr ra
nop
.align	4
.globl get_cp3_1	
get_cp3_1:
mfc3 v0, $10
jr ra
nop
.align	4
.globl get_cp3_2	
get_cp3_2:
mfc3 v0, $12
jr ra
nop
.align	4
.globl get_cp3_3	
get_cp3_3:
mfc3 v0, $14
jr ra
nop





		