// OpenCL_Assignment.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "main.h"
#include <chrono>


//------------------------------------------------------------------------------

// GPU reset?  See <http://stackoverflow.com/questions/12259044/
//                  limitations-of-work-item-load-in-gpu-cuda-opencl>

// <https://social.technet.microsoft.com/Forums/windows/en-US/
// 92a45329-3dd1-4c42-8a53-42dd232edd81/
// how-to-turn-off-timeout-detection-and-recovery-of-gpus>
//------------------------------------------------------------------------------

void Process_Serial() {
	int i, j, k;
	float result;

	for (j = 0; j < N; j++) {
		for (i = 0; i < N; i++) {
			result = 0;
			for (k = 0; k < N; k++) {
				result += A[N*i + k] * B[N*k + j];
			}
			Output_Serial[N*j + i] = result;
		}
	}
}
//------------------------------------------------------------------------------

void Process_OpenCL() {
	printf("\n");

	OpenCL_ConstantInt(3, N);

	OpenCL_WriteData(A_Buffer, N*N * sizeof(float), A);
	OpenCL_WriteData(B_Buffer, N*N * sizeof(float), B);

	OpenCL_Run(N, Local_Size);

	OpenCL_ReadData(Output_Buffer, N*N * sizeof(float), Output_OpenCL);
}
//------------------------------------------------------------------------------

void Compare() {
	int i, j;

	printf("A:\n");
	for (j = 0; j < N; j++) {
		for (i = 0; i < N; i++) {
			printf("\t%g", A[N*j + i]);
		}
		printf("\n");
	}

	printf("B:\n");
	for (j = 0; j < N; j++) {
		for (i = 0; i < N; i++) {
			printf("\t%g", B[N*j + i]);
		}
		printf("\n");
	}

	printf("Output_Serial:\n");
	for (j = 0; j < N; j++) {
		for (i = 0; i < N; i++) {
			printf("\t%g", Output_Serial[N*j + i]);
		}
		printf("\n");
	}

	printf("Output_OpenCL:\n");
	for (j = 0; j < N; j++) {
		for (i = 0; i < N; i++) {
			printf("\t%g", Output_OpenCL[N*j + i]);
		}
		printf("\n");
	}
}
//------------------------------------------------------------------------------

void Fill(float* A) {
	int i, j;

	for (j = 0; j < N; j++) {
		for (i = 0; i < N; i++) {
			A[N*j + i] = (rand()%20);
		}
	}
}
//------------------------------------------------------------------------------

int main() {
	// Initialise OpenCL
	if (
		!OpenCL_Init("NVIDIA") && // nVidia
		!OpenCL_Init("Advanced Micro Devices") && // AMD
		!OpenCL_Init(0)    // Default
		) {
		printf("Error: Cannot initialise OpenCL.\n");
		return 1;
	}

	// Load a kernel
	if (!OpenCL_LoadKernel("OpenCL/Kernel.cl", "Multiply")) return 1;

	// Files to write results on
	mulFile.open("Multiplication_Results.txt", std::ios::out | std::ios::app);
	


	size_t BufferSize;

	N = 25;

	BufferSize = N * N * sizeof(float);

	// Allocate CPU RAM
	A = (float*)malloc(BufferSize);
	B = (float*)malloc(BufferSize);

	Output_Serial = (float*)malloc(BufferSize);
	Output_OpenCL = (float*)malloc(BufferSize);

	Fill(A);
	Fill(B);

	// Allocate GPU RAM
	OpenCL_PrepareLocalSize(N, Local_Size);
	A_Buffer = OpenCL_CreateBuffer(0, CL_MEM_READ_ONLY, BufferSize);
	B_Buffer = OpenCL_CreateBuffer(1, CL_MEM_READ_ONLY, BufferSize);
	Output_Buffer = OpenCL_CreateBuffer(2, CL_MEM_WRITE_ONLY, BufferSize);

	// Process the matrices
	CPU_tic();
	Process_Serial();
	mulFile << "Serial: " << CPU_toc() << " ms\n";

	CPU_tic();
	Process_OpenCL();
	mulFile << "OpenCL: " << CPU_toc() << " ms\n";

	// Compare results
	//Compare();

	// Clean-up
	free(A);
	free(B);
	free(Output_Serial);
	free(Output_OpenCL);

	OpenCL_FreeBuffer(A_Buffer);
	OpenCL_FreeBuffer(B_Buffer);
	OpenCL_FreeBuffer(Output_Buffer);
	OpenCL_Destroy();


	mulFile.close();
	rwFile.close();
	return 0;
}
//------------------------------------------------------------------------------

