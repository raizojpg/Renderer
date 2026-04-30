#pragma once

#include <GL/glew.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <limits>
#include <string>
#include <unordered_map>
#include <vector>

struct FrameStats {
	std::uint64_t frameIndex = 0;
	double cpuMs = 0.0;
	double gpuMs = std::numeric_limits<double>::quiet_NaN();
	std::uint32_t drawCalls = 0;
	std::unordered_map<std::string, double> gpuSectionsMs;
};

struct MetricSummary {
	double latest = 0.0;
	double average = 0.0;
	double minimum = 0.0;
	double maximum = 0.0;
	double p95 = 0.0;
	double p99 = 0.0;
	std::size_t sampleCount = 0;
};

class Profiler
{
public:
	explicit Profiler(std::size_t historySize = 240, std::size_t gpuFramesInFlight = 4);
	~Profiler();

	void setEnabled(bool enabled);
	bool isEnabled() const;

	void beginFrame();
	FrameStats endFrame();

	void beginGpuSection(const std::string& name);
	void endGpuSection();

	void incrementDrawCall(std::uint32_t count = 1);

	const FrameStats* getLatestFrame() const;
	const std::deque<FrameStats>& getHistory() const;

	MetricSummary getCpuSummary() const;
	MetricSummary getGpuSummary() const;
	MetricSummary getDrawCallSummary() const;
	MetricSummary getGpuSectionSummary(const std::string& sectionName) const;

	bool exportCsv(const std::string& filePath) const;

	bool isGpuTimingSupported() const;

	class ScopedGpuSection
	{
	public:
		ScopedGpuSection(Profiler& profiler, const char* name);
		~ScopedGpuSection();

	private:
		Profiler* profiler = nullptr;
	};

	class ScopedBinding
	{
	public:
		explicit ScopedBinding(Profiler& profiler);
		~ScopedBinding();

	private:
		Profiler* previousProfiler = nullptr;
	};

	static Profiler* GetActive();

private:
	struct GpuSection {
		std::string name;
		GLuint queryId = 0;
	};

	struct PendingFrame {
		bool pending = false;
		std::uint64_t frameIndex = 0;
		std::vector<GpuSection> sections;
	};

	using Clock = std::chrono::high_resolution_clock;

	void clearGpuState();
	void resolveReadyGpuFrames();

	GLuint getQuery();
	void storeQuery(GLuint queryId);

	bool enabled = false;
	bool gpuSupported = true;
	bool gpuSupportChecked = false;
	bool frameActive = false;
	bool gpuSectionOpen = false;

	std::size_t maxHistorySize = 240;
	std::size_t nextPendingFrameIndex = 0;

	std::uint64_t frameCounter = 0;
	std::uint32_t drawCallCounter = 0;

	Clock::time_point cpuFrameStart;
	FrameStats currentFrame;

	std::vector<GpuSection> currentSections;
	std::vector<PendingFrame> pendingFrames;
	std::vector<GLuint> freeQueries;
	std::deque<FrameStats> history;
};

#define PROFILE_GPU(PROFILER, NAME) Profiler::ScopedGpuSection profilerGpuSection_##__LINE__((PROFILER), (NAME))

namespace ProfilerGL {
	void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);
	void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instanceCount);
	void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint baseVertex);
}
