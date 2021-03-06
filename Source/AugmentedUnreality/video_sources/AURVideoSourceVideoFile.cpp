/*
Copyright 2016 Krzysztof Lis

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http ://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "AugmentedUnreality.h"
#include "AURVideoSourceVideoFile.h"

const float UAURVideoSourceVideoFile::MIN_FPS = 0.1;
const float UAURVideoSourceVideoFile::MAX_FPS = 60.0;

FText UAURVideoSourceVideoFile::GetSourceName() const
{
	if(!SourceName.IsEmpty())
	{
		return SourceName;
	}
	else
	{
		return FText::FromString("File " + FPaths::GetCleanFilename(VideoFile));
	}
}

bool UAURVideoSourceVideoFile::Connect()
{
	FString full_path = FPaths::GameDir() / VideoFile;

	if (!FPaths::FileExists(full_path))
	{
		UE_LOG(LogAUR, Error, TEXT("UAURVideoSourceVideoFile::Connect: File %s does not exist"), *full_path)
		return false;
	}

	bool success = OpenVideoCapture(full_path);

	Period = 1.0;
	if (success)
	{
		// Determine time needed to wait
		float fps = GetFrequency();

		UE_LOG(LogAUR, Log, TEXT("UAURVideoSourceVideoFile::Connect: Opened video file %s, reported FPS = %lf"),
			*full_path, fps)

		fps = FMath::Clamp(fps, MIN_FPS, MAX_FPS);
		Period = 1.0 / fps;

		FrameCount = FPlatformMath::RoundToInt(Capture.get(cv::CAP_PROP_FRAME_COUNT));

		LoadCalibration();
	}
	else
	{
		UE_LOG(LogAUR, Error, TEXT("UAURVideoSourceVideoFile::Connect: Failed to open video file %s"), *full_path)
	}

	return success;
}

bool UAURVideoSourceVideoFile::GetNextFrame(cv::Mat & frame)
{
	// Simulate camera delay by waiting
	FPlatformProcess::Sleep(Period);

	bool success = Capture.read(frame);

	// Loop the video
	if (Capture.get(cv::CAP_PROP_POS_FRAMES) >= FrameCount - 1)
	{
		Capture.set(cv::CAP_PROP_POS_FRAMES, 0);
	}

	return success;
}