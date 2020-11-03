/*
 * Copyright (C) 2011      David Saunders
 *               2011-2012 Matthew Wezowicz
 *
 * Written by Matthew Wezowicz <mwezz@udel.edu>
 *
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
 * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 *.
 */

/** @internal
 * @file linbox/matrix/matrixdomain/opencl-domain.inl
 * @brief NO DOC
 */

#ifndef __LINBOX_opencl_matrix_domain_INL
#define __LINBOX_opencl_matrix_domain_INL

#include <pthread.h>

#include "linbox/matrix/densematrix/blas-matrix.h"

#include "CL/cl.h"

namespace LinBox
{

	/*
	 * ******************************************************
	 * *** Specializations for BlasMatrix<Field> where    ***
	 * *** the Field is Givaro::Modular<float> or Givaro::Modular<double> ***
	 * ******************************************************
	 */

	/*
	 * Specialization of Mul for
	 * multiplying two general dense matrices
	 * over a Givaro::Modular<double> Field.
	 * C = A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::mul(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[0];
		kernelsAvailable &= dpKernelsAvailable[1];
		kernelsAvailable &= dpKernelsAvailable[2];
		kernelsAvailable &= dpKernelsAvailable[3];
		kernelsAvailable &= dpKernelsAvailable[8];
		kernelsAvailable &= dpKernelsAvailable[9];
		kernelsAvailable &= dpKernelsAvailable[10];
		kernelsAvailable &= dpKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMul<Givaro::Modular<double>,Operand1,Operand2,Operand3>()(
				_F,
				C,
				A,
				B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			C,
			A,
			B,
			VC,
			VA,
			VB);

		//Break out the partitioned dimensions
		int CBlocksX = partitionDims.at(0);
		int CBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		double p = (double) _F.characteristic();

		//Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^53
		cl_kernel selectedMulKernel;
		cl_kernel selectedAxpyKernel;
		if(p <= (1 << 21)){
			selectedMulKernel = dpKernels[3];
			selectedAxpyKernel = dpKernels[11];
		}
		else if(p <= (1 << 24)){
			selectedMulKernel = dpKernels[2];
			selectedAxpyKernel = dpKernels[10];
		}
		else if(p <= (1 << 25)){
			selectedMulKernel = dpKernels[1];
			selectedAxpyKernel = dpKernels[9];
		}
		else{
			selectedMulKernel = dpKernels[0];
			selectedAxpyKernel = dpKernels[8];
		}

		for(int blockCol = 0; blockCol < CBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < CBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * CBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)blockCol);

				//Allocate buffers
				cl_mem bufferC = oclCreateMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SC);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand3> >(SB);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<double, cl_double>(
					bufferC,
					bufferA,
					bufferB,
					widthA,
					heightA,
					widthB,
					p,
					selectedMulKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = oclCreateMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SC);

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));

					bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<double,cl_double>(tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedAxpyKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SC = oclReadMatrixBuffer<cl_double,SubmatrixAdapter<Operand1> >(
					bufferC,
					SC);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);

			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return C;
	}

	/*
	 * Specialization of Mul for
	 * multiplying two general dense matrices
	 * over a Givaro::Modular<float> Field.
	 * C = A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::mul(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[0];
		kernelsAvailable &= spKernelsAvailable[1];
		kernelsAvailable &= spKernelsAvailable[2];
		kernelsAvailable &= spKernelsAvailable[3];
		kernelsAvailable &= spKernelsAvailable[8];
		kernelsAvailable &= spKernelsAvailable[9];
		kernelsAvailable &= spKernelsAvailable[10];
		kernelsAvailable &= spKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMul<Givaro::Modular<float>,Operand1,Operand2,Operand3>()(
				_F,
				C,
				A,
				B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			C,
			A,
			B,
			VC,
			VA,
			VB);

		//Break out the partitioned dimensions
		int CBlocksX = partitionDims.at(0);
		int CBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		float p = (float) _F.characteristic();

		//Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^53
		cl_kernel selectedMulKernel;
		cl_kernel selectedAxpyKernel;
		if(p <= (1 << 21)){
			selectedMulKernel = spKernels[3];
			selectedAxpyKernel = spKernels[11];
		}
		else if(p <= (1 << 24)){
			selectedMulKernel = spKernels[2];
			selectedAxpyKernel = spKernels[10];
		}
		else if(p <= (1 << 25)){
			selectedMulKernel = spKernels[1];
			selectedAxpyKernel = spKernels[9];
		}
		else{
			selectedMulKernel = spKernels[0];
			selectedAxpyKernel = spKernels[8];
		}

		for(int blockCol = 0; blockCol < CBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < CBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * CBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)blockCol);

				//Allocate buffers
				cl_mem bufferC = oclCreateMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SC);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand3> >(SB);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<float,cl_float>(bufferC,
					bufferA,
					bufferB,
					widthA,
					heightA,
					widthB,
					p,
					selectedMulKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = oclCreateMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SC);

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<float, cl_float>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedAxpyKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SC = oclReadMatrixBuffer<cl_float,SubmatrixAdapter<Operand1> >(
					bufferC,
					SC);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);

			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return C;
	}

	/*
	 * Specialization of mulin_left for
	 * multiplying two general dense matrices
	 * over a Givaro::Modular<double> Field.
	 * Places result into the left matrix.
	 * A = A*B
	 */
	template <>
	template <class Operand1, class Operand2>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::mulin_left(
		Operand1& A,
		const Operand2& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[0];
		kernelsAvailable &= dpKernelsAvailable[1];
		kernelsAvailable &= dpKernelsAvailable[2];
		kernelsAvailable &= dpKernelsAvailable[3];
		kernelsAvailable &= dpKernelsAvailable[8];
		kernelsAvailable &= dpKernelsAvailable[9];
		kernelsAvailable &= dpKernelsAvailable[10];
		kernelsAvailable &= dpKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMulin<Givaro::Modular<double>,Operand1,Operand2>()(_F,A,B);
		}

		Operand1 T(A);
		return mul<Operand1,Operand1,Operand2>(A,T,B);
	}

	/*
	 * Specialization of mulin_left for
	 * multiplying two general dense matrices
	 * over a Givaro::Modular<float> Field.
	 * Places the result into the left matrix.
	 * A = A*B
	 */
	template <>
	template <class Operand1, class Operand2>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::mulin_left(
		Operand1& A,
		const Operand2& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[0];
		kernelsAvailable &= spKernelsAvailable[1];
		kernelsAvailable &= spKernelsAvailable[2];
		kernelsAvailable &= spKernelsAvailable[3];
		kernelsAvailable &= spKernelsAvailable[8];
		kernelsAvailable &= spKernelsAvailable[9];
		kernelsAvailable &= spKernelsAvailable[10];
		kernelsAvailable &= spKernelsAvailable[11];


		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMulin<Givaro::Modular<float>,Operand1,Operand2>()(_F,A,B);
		}

		Operand1 T(A);
		return mul<Operand1,Operand1,Operand2>(A,T,B);
	}

	/*
	 * Specialization of mulin_right for
	 * multiplying two general dense matrices
	 * over a Givaro::Modular<double> Field.
	 * Places the result into the right matrix.
	 * B = A*B
	 */
	template <>
	template <class Operand1, class Operand2>
	Operand2& OpenCLMatrixDomain<Givaro::Modular<double> >::mulin_right(
		const Operand1& A,
		Operand2& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[0];
		kernelsAvailable &= dpKernelsAvailable[1];
		kernelsAvailable &= dpKernelsAvailable[2];
		kernelsAvailable &= dpKernelsAvailable[3];
		kernelsAvailable &= dpKernelsAvailable[8];
		kernelsAvailable &= dpKernelsAvailable[9];
		kernelsAvailable &= dpKernelsAvailable[10];
		kernelsAvailable &= dpKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMulin<Givaro::Modular<double>,Operand2,Operand1>()(_F,A,B);
		}

		Operand2 T(B);
		return mul<Operand2,Operand1,Operand2>(B,A,T);
	}

	/*
	 * Specialization of mulin_right for
	 * multiplying two general dense matrices
	 * over a Givaro::Modular<float> Field.
	 * Places the result into the right matrix.
	 * B = A*B
	 */
	template <>
	template <class Operand1, class Operand2>
	Operand2& OpenCLMatrixDomain<Givaro::Modular<float> >::mulin_right(
		const Operand1& A,
		Operand2& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[0];
		kernelsAvailable &= spKernelsAvailable[1];
		kernelsAvailable &= spKernelsAvailable[2];
		kernelsAvailable &= spKernelsAvailable[3];
		kernelsAvailable &= spKernelsAvailable[8];
		kernelsAvailable &= spKernelsAvailable[9];
		kernelsAvailable &= spKernelsAvailable[10];
		kernelsAvailable &= spKernelsAvailable[11];


		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMulin<Givaro::Modular<float>,Operand2,Operand1>()(_F,A,B);
		}

		Operand2 T(B);
		return mul<Operand2,Operand1,Operand2>(B,A,T);
	}

	/*
	 * Specialization of general matrix-matrix multiplication and
	 * addition with scaling over a Givaro::Modular<double> Field
	 * D = beta.C + alpha.A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::muladd(
		Operand1& D,
		const double& beta,
		const Operand1& C,
		const double& alpha,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[4];
		kernelsAvailable &= dpKernelsAvailable[5];
		kernelsAvailable &= dpKernelsAvailable[6];
		kernelsAvailable &= dpKernelsAvailable[7];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<double>,
				Operand1,
				Operand2,
				Operand3>()(D,beta,C,alpha,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		double p = (double) _F.characteristic();
		double tempAlpha = fmod(alpha, p);
		double tempBeta = fmod(beta, p);

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^53
		cl_kernel selectedKernel;
		if(p <= (1 << 21)){
			selectedKernel = dpKernels[7];
		}
		else if(p <=(1 << 24)){
			selectedKernel = dpKernels[6];
		}
		else if(p <=(1 << 25)){
			selectedKernel = dpKernels[5];
		}
		else{
			selectedKernel = dpKernels[4];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<double,cl_double>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					tempAlpha,
					tempBeta,
					widthA,
					heightA,
					widthB,
					p,
					selectedKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<double,cl_double>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						tempAlpha,
						_F.one,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_double,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);

			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of general matrix-matrix multiplication and
	 * addition with scaling over a Givaro::Modular<float> Field
	 * D = beta.C + alpha.A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::muladd(
		Operand1& D,
		const float& beta,
		const Operand1& C,
		const float& alpha,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[4];
		kernelsAvailable &= spKernelsAvailable[5];
		kernelsAvailable &= spKernelsAvailable[6];
		kernelsAvailable &= spKernelsAvailable[7];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<float>,
				Operand1,
				Operand2,
				Operand3>()(D,beta,C,alpha,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		float p = (float) _F.characteristic();
		float tempAlpha = fmodf(alpha, p);
		float tempBeta = fmodf(beta, p);

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^23
		cl_kernel selectedKernel;
		if(p <= (1 << 7)){
			selectedKernel = spKernels[7];
		}
		else if(p <=(1 << 9)){
			selectedKernel = spKernels[6];
		}
		else if(p <=(1 << 10)){
			selectedKernel = spKernels[5];
		}
		else{
			selectedKernel = spKernels[4];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<float,cl_float>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					tempAlpha,
					tempBeta,
					widthA,
					heightA,
					widthB,
					p,
					selectedKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX *sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<float,cl_float>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						tempAlpha,
						_F.one,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_float,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);

			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of general matrix-matrix multiplication and
	 * addition with scaling over a Givaro::Modular<double> Field
	 * Places the results into the first genreral dense matrix
	 * C = beta.C + alpha.A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::muladdin(
		const double& beta,
		Operand1& C,
		const double& alpha,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[4];
		kernelsAvailable &= dpKernelsAvailable[5];
		kernelsAvailable &= dpKernelsAvailable[6];
		kernelsAvailable &= dpKernelsAvailable[7];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<double>,
				Operand1,
				Operand2,
				Operand3>()(beta,C,alpha,A,B);
		}

		Operand1 T(C);
		return muladd<Operand1,Operand2,Operand3>(C,beta,T,alpha,A,B);
	}

	/*
	 * Specialization of general matrix-matrix multiplication and
	 * addition with scaling over a Givaro::Modular<float> Field
	 * Places the results into the first genreral dense matrix
	 * C = beta.C + alpha.A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::muladdin(
		const float& beta,
		Operand1& C,
		const float& alpha,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[4];
		kernelsAvailable &= spKernelsAvailable[5];
		kernelsAvailable &= spKernelsAvailable[6];
		kernelsAvailable &= spKernelsAvailable[7];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<float>,
				Operand1,
				Operand2,
				Operand3>()(beta,C,alpha,A,B);
		}

		Operand1 T(C);
		return muladd<Operand1,Operand2,Operand3>(C,beta,T,alpha,A,B);
	}

	/*
	 * Specialization of multiplication with scaling over
	 * a Givaro::Modular<double> Field
	 * C = alpha.A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::mul(
		Operand1& C,
		const double& alpha,
		const Operand2& A,
		const Operand3& B) const{

		return muladdin<Operand1,Operand2,Operand3>(0,C,alpha,A,B);
	}

	/*
	 * Specialization of multiplication with scaling over
	 * a Givaro::Modular<float> Field
	 * C = alpha.A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::mul(
		Operand1& C,
		const float& alpha,
		const Operand2& A,
		const Operand3& B) const{

		return muladdin<Operand1,Operand2,Operand3>(0,C,alpha,A,B);
	}

	/*
	 * Specialization of apxy for
	 * multiplying two general dense matrices
	 * and adding a third general dense matrix
	 * over a Givaro::Modular<double> Field.
	 * D = A*B + C
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::axpy(
		Operand1& D,
		const Operand2& A,
		const Operand3& B,
		const Operand1& C) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[8];
		kernelsAvailable &= dpKernelsAvailable[9];
		kernelsAvailable &= dpKernelsAvailable[10];
		kernelsAvailable &= dpKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<double>,
				Operand1,
				Operand2,
				Operand3>()(D,_F.one,C,_F.one,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		double p = (double)_F.characteristic();

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^53
		cl_kernel selectedKernel;
		if(p <= (1 << 21)){
			selectedKernel = dpKernels[11];
		}
		else if(p <= (1 << 24)){
			selectedKernel = dpKernels[10];
		}
		else if(p <= (1 << 25)){
			selectedKernel = dpKernels[9];
		}
		else{
			selectedKernel = dpKernels[8];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<double,cl_double>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					widthA,
					heightA,
					widthB,
					p,
					selectedKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<double,cl_double>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
					linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_double,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);
			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of apxy for
	 * multiplying two general dense matrices
	 * and adding a third general dense matrix
	 * over a Givaro::Modular<float> Field.
	 * D = A*B + C
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::axpy(
		Operand1& D,
		const Operand2& A,
		const Operand3& B,
		const Operand1& C) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[8];
		kernelsAvailable &= spKernelsAvailable[9];
		kernelsAvailable &= spKernelsAvailable[10];
		kernelsAvailable &= spKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<float>,
				Operand1,
				Operand2,
				Operand3>()(D,_F.one,C,_F.one,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		float p = (float)_F.characteristic();

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^23
		cl_kernel selectedKernel;
		if(p <= (1 << 7)){
			selectedKernel = spKernels[11];
		}
		else if(p <= (1 << 9)){
			selectedKernel = spKernels[10];
		}
		else if(p <= (1 << 10)){
			selectedKernel = spKernels[9];
		}
		else{
			selectedKernel = spKernels[8];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<float,cl_float>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					widthA,
					heightA,
					widthB,
					p,
					selectedKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);


				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<float,cl_float>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_float,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);
			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of apxyin for
	 * multiplying two general dense matrices
	 * and adding a third general dense matrix
	 * over a Givaro::Modular<double> Field.
	 * Places the result into the first matrix.
	 * C += A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::axpyin(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[8];
		kernelsAvailable &= dpKernelsAvailable[9];
		kernelsAvailable &= dpKernelsAvailable[10];
		kernelsAvailable &= dpKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return muladdin<Operand1,Operand2,Operand3>(_F.one,C,_F.one,A,B);
		}

		Operand1 T(C);
		return axpy<Operand1,Operand2,Operand3>(C,A,B,T);
	}

	/*
	 * Specialization of apxyin for
	 * multiplying two general dense matrices
	 * and adding a third general dense matrix
	 * over a Givaro::Modular<float> Field.
	 * Places the result into the first matrix.
	 * C += A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::axpyin(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[8];
		kernelsAvailable &= spKernelsAvailable[9];
		kernelsAvailable &= spKernelsAvailable[10];
		kernelsAvailable &= spKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return muladdin<Operand1,Operand2,Operand3>(_F.one,C,_F.one,A,B);
		}

		Operand1 T(C);
		return axpy<Operand1,Operand2,Operand3>(C,A,B,T);
	}

	/*
	 * Specialization of maxpy for
	 * multiplying two general dense matrices
	 * and subtracts it from a third general dense matrix
	 * over a Givaro::Modular<double> Field.
	 * D = C - A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::maxpy(
		Operand1& D,
		const Operand2& A,
		const Operand3& B,
		const Operand1& C) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[12];
		kernelsAvailable &= dpKernelsAvailable[13];
		kernelsAvailable &= dpKernelsAvailable[14];
		kernelsAvailable &= dpKernelsAvailable[15];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<double>,
				Operand1,
				Operand2,
				Operand3>()(D,_F.one,C,_F.mOne,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		double p = (double)_F.characteristic();

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^53
		cl_kernel selectedKernel;
		if(p <= (1 << 21)){
			selectedKernel = dpKernels[15];
		}
		else if(p <= (1 << 24)){
			selectedKernel = dpKernels[14];
		}
		else if(p <= (1 << 25)){
			selectedKernel = dpKernels[13];
		}
		else{
			selectedKernel = dpKernels[12];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<double,cl_double>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					widthA,
					heightA,
					widthB,
					p,
					selectedKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<double,cl_double>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_double,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);
			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of maxpy for
	 * multiplying two general dense matrices
	 * and subtracts it from a third general dense matrix
	 * over a Givaro::Modular<float> Field.
	 * D = C - A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::maxpy(
		Operand1& D,
		const Operand2& A,
		const Operand3& B,
		const Operand1& C) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[12];
		kernelsAvailable &= spKernelsAvailable[13];
		kernelsAvailable &= spKernelsAvailable[14];
		kernelsAvailable &= spKernelsAvailable[15];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<float>,
				Operand1,
				Operand2,
				Operand3>()(D,_F.one,C,_F.mOne,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		float p = (float) _F.characteristic();

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^23
		cl_kernel selectedKernel;;
		if(p <= (1 << 7)){
			selectedKernel = spKernels[15];
		}
		else if(p <= (1 << 9)){
			selectedKernel = spKernels[14];
		}
		else if(p <= (1 << 10)){
			selectedKernel = spKernels[13];
		}
		else{
			selectedKernel = spKernels[12];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<float,cl_float>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					widthA,
					heightA,
					widthB,
					p,
					selectedKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<float,cl_float>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_float,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);
			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of maxpyin for
	 * multiplying two general dense matrices
	 * and subtracts it from a third general dense matrix
	 * over a Givaro::Modular<double> Field.
	 * Places the results into the first gernal dense matrix.
	 * C -= A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::maxpyin(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[12];
		kernelsAvailable &= dpKernelsAvailable[13];
		kernelsAvailable &= dpKernelsAvailable[14];
		kernelsAvailable &= dpKernelsAvailable[15];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return muladdin<Operand1,Operand2,Operand3>(_F.one,C,_F.mOne,A,B);
		}

		Operand1 T(C);
		return maxpy<Operand1,Operand2,Operand3>(C,A,B,T);
	}

	/*
	 * Specialization of maxpyin for
	 * multiplying two general dense matrices
	 * and subtracts it from a third general dense matrix
	 * over a Givaro::Modular<float> Field.
	 * Places the results into the first gernal dense matrix.
	 * C -= A*B
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::maxpyin(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[12];
		kernelsAvailable &= spKernelsAvailable[13];
		kernelsAvailable &= spKernelsAvailable[14];
		kernelsAvailable &= spKernelsAvailable[15];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return muladdin<Operand1,Operand2,Operand3>(_F.one,C,_F.mOne,A,B);
		}

		Operand1 T(C);
		return maxpy<Operand1,Operand2,Operand3>(C,A,B,T);
	}

	/*
	 * Specialization of axmy for
	 * multiplying two general dense matrices
	 * and subtracts a third general dense matrix from it
	 * over a Givaro::Modular<double> Field.
	 * D = A*B - C
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::axmy(
		Operand1& D,
		const Operand2& A,
		const Operand3& B,
		const Operand1& C) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[16];
		kernelsAvailable &= dpKernelsAvailable[17];
		kernelsAvailable &= dpKernelsAvailable[18];
		kernelsAvailable &= dpKernelsAvailable[19];
		kernelsAvailable &= dpKernelsAvailable[8];
		kernelsAvailable &= dpKernelsAvailable[9];
		kernelsAvailable &= dpKernelsAvailable[10];
		kernelsAvailable &= dpKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<double>,
				Operand1,
				Operand2,
				Operand3>()(D,_F.mOne,C,_F.one,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		double p = (double)_F.characteristic();

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^53
		cl_kernel selectedAxmyKernel;
		cl_kernel selectedAxpyKernel;
		if(p <= (1 << 21)){
			selectedAxmyKernel = dpKernels[19];
			selectedAxpyKernel = dpKernels[11];
		}
		else if(p <= (1 << 24)){
			selectedAxmyKernel = dpKernels[18];
			selectedAxpyKernel = dpKernels[10];
		}
		else if(p <= (1 << 25)){
			selectedAxmyKernel = dpKernels[17];
			selectedAxpyKernel = dpKernels[9];
		}
		else{
			selectedAxmyKernel = dpKernels[16];
			selectedAxpyKernel = dpKernels[8];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_double,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<double,cl_double>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					widthA,
					heightA,
					widthB,
					p,
					selectedAxmyKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_double,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<double,cl_double>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedAxpyKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_double,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);
			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of axmy for
	 * multiplying two general dense matrices
	 * and subtracts a third general dense matrix from it
	 * over a Givaro::Modular<float> Field.
	 * D = A*B - C
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::axmy(
		Operand1& D,
		const Operand2& A,
		const Operand3& B,
		const Operand1& C) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[16];
		kernelsAvailable &= spKernelsAvailable[17];
		kernelsAvailable &= spKernelsAvailable[18];
		kernelsAvailable &= spKernelsAvailable[19];
		kernelsAvailable &= spKernelsAvailable[8];
		kernelsAvailable &= spKernelsAvailable[9];
		kernelsAvailable &= spKernelsAvailable[10];
		kernelsAvailable &= spKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return BlasMatrixDomainMulAdd<//Givaro::Modular<float>,
				Operand1,
				Operand2,
				Operand3>()(D,_F.mOne,C,_F.one,A,B);
		}

		//Check dimensions
		linbox_check( A.coldim() == B.rowdim());
		linbox_check( C.rowdim() == A.rowdim());
		linbox_check( C.coldim() == B.coldim());
		linbox_check( D.rowdim() == C.rowdim());
		linbox_check( D.coldim() == C.coldim());

		//Lock the device
		pthread_mutex_lock(deviceLock);

		//Partition the input matrices into chuncks that can fit onto the device
		std::vector<SubmatrixAdapter<Operand1> > VD;
		std::vector<SubmatrixAdapter<Operand2> > VA;
		std::vector<SubmatrixAdapter<Operand3> > VB;
		std::vector<SubmatrixAdapter<Operand1> > VC;
		std::vector<int> partitionDims = oclPartition<Operand1,Operand2,Operand3>(
			D,
			A,
			B,
			C,
			VD,
			VA,
			VB,
			VC);

		//Break out the partitioned dimensions
		int DBlocksX = partitionDims.at(0);
		int DBlocksY = partitionDims.at(1);
		int ABlocksX = partitionDims.at(2);
		//int ABlocksY = partitionDims.at(3);
		int BBlocksX = partitionDims.at(4);
		//nt BBlocksY = partitionDims.at(5);

		float p = (float)_F.characteristic();

		// Select OpenCL kernel based on the size of the modulus factor for
		//maximum performance
		//p^2 * n < 2^23
		cl_kernel selectedAxmyKernel;
		cl_kernel selectedAxpyKernel;
		if(p <= (1 << 7)){
			selectedAxmyKernel = spKernels[19];
			selectedAxpyKernel = spKernels[11];
		}
		else if(p <= (1 << 9)){
			selectedAxmyKernel = spKernels[18];
			selectedAxpyKernel = spKernels[10];
		}
		else if(p <= (1 << 10)){
			selectedAxmyKernel = spKernels[17];
			selectedAxpyKernel = spKernels[9];
		}
		else{
			selectedAxmyKernel = spKernels[16];
			selectedAxpyKernel = spKernels[8];
		}

		for(int blockCol = 0; blockCol < DBlocksX; blockCol++){
			for(int blockRow = 0; blockRow < DBlocksY; blockRow++){

				SubmatrixAdapter<Operand1> SD = VD.at((size_t)(blockRow * DBlocksX + blockCol));
				SubmatrixAdapter<Operand2> SA = VA.at((size_t)(blockRow * ABlocksX));
				SubmatrixAdapter<Operand3> SB = VB.at((size_t)(blockCol));
				SubmatrixAdapter<Operand1> SC = VC.at((size_t)(blockRow * DBlocksX + blockCol));

				//Allocate buffers
				cl_mem bufferD = oclCreateMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SD);
				cl_mem bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand2> >(SA);
				cl_mem bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand3> >(SB);
				cl_mem bufferC = oclCreateAndLoadMatrixBuffer<cl_float,
					SubmatrixAdapter<Operand1> >(SC);

				//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
				int widthA  = (int) ((SA.coldim() + 15) / 16) * 16;
				int heightA = (int) ((SA.rowdim() + 15) / 16) * 16;
				int widthB  = (int) ((SB.coldim() + 15) / 16) * 16;

				//Call the kernel
				oclCallKernel<float,cl_float>(
					bufferD,
					bufferA,
					bufferB,
					bufferC,
					widthA,
					heightA,
					widthB,
					p,
					selectedAxmyKernel);

				//Block until kernel finishes
				cl_int tempErrcode;
				tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

				//Create temporary accumulation buffer
				cl_mem tempBuffer = bufferC;
				bufferC = bufferD;

				for(int sharedDim = 1; sharedDim < ABlocksX; sharedDim++){
					//Load next blocks onto the device
					SA = VA.at((size_t)(blockRow * ABlocksX + sharedDim));
					SB = VB.at((size_t)(blockCol + BBlocksX * sharedDim));
					bufferA = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand2> >(SA);
					bufferB = oclCreateAndLoadMatrixBuffer<cl_float,
						SubmatrixAdapter<Operand3> >(SB);

					//((A.coldim() / 16) + (A.coldim() % 16 == 0 ? 0 : 1)) * 16
					int widthA_  = (int) ((SA.coldim() + 15) / 16) * 16;
					int heightA_ = (int) ((SA.rowdim() + 15) / 16) * 16;
					int widthB_  = (int) ((SB.coldim() + 15) / 16) * 16;

					//Call the kernel
					oclCallKernel<float,cl_float>(
						tempBuffer,
						bufferA,
						bufferB,
						bufferC,
						widthA_,
						heightA_,
						widthB_,
						p,
						selectedAxpyKernel);

					//Block until kernel finishes
					tempErrcode = clFinish(commandQue);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Delete OpenCL buffers
					tempErrcode = clReleaseMemObject(bufferA);
				linbox_check(tempErrcode == CL_SUCCESS);

					tempErrcode = clReleaseMemObject(bufferB);
				linbox_check(tempErrcode == CL_SUCCESS);

					//Shuffle buffer variables
					cl_mem placeHolder = bufferC;
					bufferC = tempBuffer;
					tempBuffer = placeHolder;
				}

				//Read back buffer
				SD = oclReadMatrixBuffer<cl_float,SubmatrixAdapter<Operand1> >(
					bufferC,
					SD);

				//Delete OpenCL buffers
				tempErrcode = clReleaseMemObject(bufferC);
				linbox_check(tempErrcode == CL_SUCCESS);

				tempErrcode = clReleaseMemObject(tempBuffer);
				linbox_check(tempErrcode == CL_SUCCESS);
			}
		}

		//Unlock the device
		pthread_mutex_unlock(deviceLock);

		return D;
	}

	/*
	 * Specialization of axmyin for
	 * multiplying two general dense matrices
	 * and subtracts a third general dense matrix from it
	 * over a Givaro::Modular<double> Field.
	 * Places the results into the first general dense matrix
	 * C = A*B - C
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<double> >::axmyin(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = dpKernelsAvailable[16];
		kernelsAvailable &= dpKernelsAvailable[17];
		kernelsAvailable &= dpKernelsAvailable[18];
		kernelsAvailable &= dpKernelsAvailable[19];
		kernelsAvailable &= dpKernelsAvailable[8];
		kernelsAvailable &= dpKernelsAvailable[9];
		kernelsAvailable &= dpKernelsAvailable[10];
		kernelsAvailable &= dpKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !doubleSupported || !kernelsAvailable){
			return muladdin<Operand1,Operand2,Operand3>(_F.mOne,C,_F.one,A,B);
		}

		Operand1 T(C);
		return axmy<Operand1,Operand2,Operand3>(C,A,B,T);
	}

	/*
	 * Specialization of axmyin for
	 * multiplying two general dense matrices
	 * and subtracts a third general dense matrix from it
	 * over a Givaro::Modular<float> Field.
	 * Places the results into the first general dense matrix
	 * C = A*B - C
	 */
	template <>
	template <class Operand1, class Operand2, class Operand3>
	Operand1& OpenCLMatrixDomain<Givaro::Modular<float> >::axmyin(
		Operand1& C,
		const Operand2& A,
		const Operand3& B) const{

		//Check if kernels are available
		bool kernelsAvailable = spKernelsAvailable[16];
		kernelsAvailable &= spKernelsAvailable[17];
		kernelsAvailable &= spKernelsAvailable[18];
		kernelsAvailable &= spKernelsAvailable[19];
		kernelsAvailable &= spKernelsAvailable[8];
		kernelsAvailable &= spKernelsAvailable[9];
		kernelsAvailable &= spKernelsAvailable[10];
		kernelsAvailable &= spKernelsAvailable[11];

		//If it is not capable or not setup properly use default implementation
		if(!setupCorrect || !kernelsAvailable){
			return muladdin<Operand1,Operand2,Operand3>(_F.mOne,C,_F.one,A,B);
		}

		Operand1 T(C);
		return axmy<Operand1,Operand2,Operand3>(C,A,B,T);
	}

} //end of namespace LinBox

#endif // __LINBOX_opencl_matrix_domain_INL

// Local Variables:
// mode: C++
// tab-width: 8
// indent-tabs-mode: nil
// c-basic-offset: 8
// End:
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
