/*
*	@file
*	@author  Thomas Kroes <t.kroes at tudelft.nl>
*	@version 1.0
*	
*	@section LICENSE
*	
*	Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
*	
*	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
*	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
*	Neither the name of the TU Delft nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
*	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include "buffer\buffers.h"
#include "color\color.h"
#include "core\rng.h"

namespace ExposureRender
{

/*! Film class */
class EXPOSURE_RENDER_DLL Film
{
public:
	/*! Film constructor
		@param[in] Resolution Resolution of the film plane
	*/
	HOST Film(const Vec2i& Resolution) :
		Resolution(),
		IterationEstimateHDR(),
		IterationEstimateLDR(),
		IterationEstimateTempFilterLDR(),
		AccumulatedEstimate(),
		CudaRunningEstimate(),
		HostRunningEstimate(),
		RandomSeeds1(),
		RandomSeeds2(),
		HostRandomSeeds1(),
		HostRandomSeeds2(),
		NoEstimates(1)
	{
		this->Resize(Resolution);

		this->GaussianFilterWeights[0]	= 0.24197072451914536f;
		this->GaussianFilterWeights[1]	= 0.39894228040143270f;
		this->GaussianFilterWeights[2]	= 0.24197072451914536f;
	}

	/*! Resize the film
		@param[in] Resolution New resolution of the film
	*/
	HOST void Resize(const Vec2i& Resolution)
	{
		if (this->Resolution == Resolution)
			return;
		
		this->Resolution = Resolution;

		this->IterationEstimateHDR.Resize(this->Resolution);
		this->IterationEstimateLDR.Resize(this->Resolution);
		this->IterationEstimateTempFilterLDR.Resize(this->Resolution);
		this->AccumulatedEstimate.Resize(this->Resolution);
		this->CudaRunningEstimate.Resize(this->Resolution);
		this->HostRunningEstimate.Resize(this->Resolution);
		this->RandomSeeds1.Resize(this->Resolution);
		this->RandomSeeds2.Resize(this->Resolution);
		this->HostRandomSeeds1.Resize(this->Resolution);
		this->HostRandomSeeds2.Resize(this->Resolution);
		
		this->RandomSeeds1.FromHost(this->HostRandomSeeds1.GetData());
		this->RandomSeeds2.FromHost(this->HostRandomSeeds2.GetData());

		this->Grid.x = (int)ceilf((float)this->Resolution[0] / (float)this->Block.x);
		this->Grid.y = (int)ceilf((float)this->Resolution[1] / (float)this->Block.y);
	}

	/*! Restarts the mc algorithm */
	HOST void Restart()
	{
		this->NoEstimates = 1;
	}

	/*! Returns the film resolution
		@return Film resolution
	*/
	HOST_DEVICE Vec2i GetResolution() const
	{
		return this->Resolution;
	}

	/*! Returns the film width
		@return Film width
	*/
	HOST_DEVICE int GetWidth() const
	{
		return this->Resolution[0];
	}

	/*! Returns the film height
		@return Film height
	*/
	HOST_DEVICE int GetHeight() const
	{
		return this->Resolution[1];
	}

	/*! Returns the hdr iteration estimate
		@return HDR iteration estimate
	*/
	HOST_DEVICE CudaBuffer2D<ColorXYZAf>& GetIterationEstimateHDR()
	{
		return this->IterationEstimateHDR;
	}

	/*! Returns the ldr iteration estimate
		@return LDR iteration estimate
	*/
	HOST_DEVICE CudaBuffer2D<ColorRGBAuc>& GetIterationEstimateLDR()
	{
		return this->IterationEstimateLDR;
	}

	/*! Returns the filter temporary ldr iteration estimate
		@return Temporary fitler ldr iteration estimate
	*/
	HOST_DEVICE CudaBuffer2D<ColorRGBAuc>& GetIterationEstimateTempFilterLDR()
	{
		return this->IterationEstimateTempFilterLDR;
	}

	/*! Returns the accumulated estimate
		@return Accumulated estimate
	*/
	HOST_DEVICE CudaBuffer2D<ColorRGBAul>& GetAccumulatedEstimate()
	{
		return this->AccumulatedEstimate;
	}

	/*! Returns the cuda running estimate
		@return Cuda running estimate
	*/
	HOST_DEVICE CudaBuffer2D<ColorRGBuc>& GetCudaRunningEstimate()
	{
		return this->CudaRunningEstimate;
	}

	/*! Returns the host running estimate
		@return Host running estimate
	*/
	HOST_DEVICE HostBuffer2D<ColorRGBuc>& GetHostRunningEstimate()
	{
		return this->HostRunningEstimate;
	}

	/*! Returns the first random seeds buffer
		@return First random seeds buffer
	*/
	HOST_DEVICE CudaRandomSeedBuffer2D& GetRandomSeeds1()
	{
		return this->RandomSeeds1;
	}

	/*! Returns the second random seeds buffer
		@return Second random seeds buffer
	*/
	HOST_DEVICE CudaRandomSeedBuffer2D& GetRandomSeeds2()
	{
		return this->RandomSeeds2;
	}

	/*! Returns the first host random seeds buffer
		@return First host random seeds buffer
	*/
	HOST_DEVICE HostRandomSeedBuffer2D& GetHostRandomSeeds1()
	{
		return this->HostRandomSeeds1;
	}

	/*! Returns the second host random seeds buffer
		@return Second host random seeds buffer
	*/
	HOST_DEVICE HostRandomSeedBuffer2D& GetHostRandomSeeds2()
	{
		return this->HostRandomSeeds2;
	}

	/*! Returns the gaussian filter weights for a 3 x 3 kernel
		@return Gaussian filter weights
	*/
	HOST_DEVICE float* GetGaussianFilterWeights()
	{
		return this->GaussianFilterWeights;
	}

	/*! Returns the random number generator for the specified pixel coordinates
		@param[in] Pixel Pixel coordinates
		@return Random number generator
	*/
	HOST_DEVICE RNG GetRandomNumberGenerator(const Vec2i& Pixel)
	{
		return RNG(&this->RandomSeeds1(Pixel[0], Pixel[1]), &this->RandomSeeds2(Pixel[0], Pixel[1]));
	}

	/*! Returns the no estimates rendered so far
		@return Number of estimates
	*/
	HOST_DEVICE int GetNoEstimates() const
	{
		return this->NoEstimates;
	}

	/*! Incrementes the number of estimates rendered so far */
	HOST_DEVICE void IncrementNoEstimates()
	{
		this->NoEstimates++;
	}

	dim3							Block;								/*! Cuda thread block size */
	dim3							Grid;								/*! Cuda launch grid size */

protected:
	Vec2i							Resolution;							/*! Resolution of the frame buffer */
	CudaBuffer2D<ColorXYZAf>		IterationEstimateHDR;				/*! High dynamic range estimate from a single iteration of the mc algorithm */
	CudaBuffer2D<ColorRGBAuc>		IterationEstimateLDR;				/*! Low dynamic range (tone mapped) estimate from a single iteration of the mc algorithm */
	CudaBuffer2D<ColorRGBAuc>		IterationEstimateTempFilterLDR;		/*! Low dynamic range (tone mapped) temporary estimate from a single iteration of the mc algorithm (for filtering purposes)*/
	CudaBuffer2D<ColorRGBAul>		AccumulatedEstimate;				/*! Accumulation buffer */
	CudaBuffer2D<ColorRGBuc>		CudaRunningEstimate;				/*! Integrated estimate in cuda memory space*/
	HostBuffer2D<ColorRGBuc>		HostRunningEstimate;				/*! Integrated estimate in host memory space */
	CudaRandomSeedBuffer2D			RandomSeeds1;						/*! First random seed buffer */
	CudaRandomSeedBuffer2D			RandomSeeds2;						/*! Second random seed buffer */
	HostRandomSeedBuffer2D			HostRandomSeeds1;					/*! First host random seed buffer */
	HostRandomSeedBuffer2D			HostRandomSeeds2;					/*! Second host random seed buffer */
	float							GaussianFilterWeights[3];			/*! Gaussian filtering weights */
	int								NoEstimates;						/*! Number of estimates rendererd so far */
};

}