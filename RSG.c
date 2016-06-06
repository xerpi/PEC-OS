#include "RSG.h"

void RSG() {

	int exceptionID;
	__asm__(
		"$push  R0, R1, R2, R3, R4, R6"	//salvem registres a la ila del proces
		"$movei R0, %2"
		"st 0(R0), R7"					//posem el SP al currentTaskStruct
		"$MOVEI R7, %1"
		"ld R7, 0(R7)"					//cargem SP de system
		"rds    %0, S2"					//llegim  exceptionID
		: "=r" (exceptionID)
		: "i"  (systemSPadress)
		: "i"  (currentSPaddress)
	);
	int pid = getPID();
	switch (exceptionID) {

	case ilegalInstruction:
		killProcess(pid);
		break;

	case accesNotAlign:
		killProcess(pid);
		break;

	case divZero:
		killProcess(pid);
		break;

	case missTLBi:
		int vTAG;
		__asm__(
		  	"rds %0, S3"
		  	: "=r" (vTAG)
		);
		handleMissTLB(instruction, pid, vTAG);
		break;

	case missTLBd:
		int vTAG;
		__asm__(
		  	"rds %0, S3"
		  	: "=r" (vTAG)
		);
		handleMissTLB(data, pid, vTAG);
		break;

	case invTLBi:
		int vTAG;
		__asm__(
		  	"rds %0, S3"
		  	: "=r" (vTAG)
		);
		handleInvalidTLB(instruction, pid, vTAG);
		break;

	case invTLBd:
		int vTAG;
		__asm__(
		  	"rds %0, S3"
		  	: "=r" (vTAG)
		);
		handleInvalidTLB(data, pid, vTAG);
		break;

	case sysTLBi:
		killProcess(pid);
		break;

	case sysTLBd:
		killProcess(pid);
		break;

	case readOnlyTLB:
		killProcess(pid);
		break;

	case sysInstruction:
		killProcess(pid);
		break;

	case systemCall:
		int systemCallID;
		__asm__(
		  	"rds    %0, S3"
		  	: "=r" (systemCallID)
		);
		handleSystemCall(pid, systemCallID);
		break;

	case interruption:
		int interruptID;
		__asm__(
		  	"rds    %0, S3"
		  	: "=r" (interruptID)
		);
		handleInterruption(pid, interruptID);
		break;
	}
	__asm__(
		"$movei R0, %0"
		"st 0(R0), R7"		//guardem system SP
		"$movei R0, %1"
		"ld R7, 0 (R0)"		//carguem current SP
		"$pop  R0, R1, R2, R3, R4, R6"
		"reti"
		: "i"  (systemSPadress)
		: "i"  (currentSPaddress)
	);
}
