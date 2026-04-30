#include "Profiler.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>

constexpr double NANOSECONDS_TO_MILLISECONDS = 1.0 / 1000000.0;

thread_local Profiler* activeProfiler = nullptr;

std::string escapeCsv(const std::string& value) {
	bool needsQuotes = false;
	for (char ch : value) {
		if (ch == ',' || ch == '"' || ch == '\n') {
			needsQuotes = true;
			break;
		}
	}

	if (!needsQuotes) {
		return value;
	}

	std::string result = "\"";
	for (char ch : value) {
		if (ch == '"') {
			result += "\"\"";
		}
		else {
			result += ch;
		}
	}

	result += '"';
	return result;
}

MetricSummary buildSummaryFromValues(const std::vector<double>& values) {
	MetricSummary summary{};
	summary.sampleCount = values.size();

	if (values.empty()) {
		return summary;
	}

	summary.latest = values.back();
	summary.minimum = values.front();
	summary.maximum = values.front();

	double total = 0.0;
	for (double value : values) {
		total += value;

		if (value < summary.minimum) {
			summary.minimum = value;
		}

		if (value > summary.maximum) {
			summary.maximum = value;
		}
	}

	summary.average = total / static_cast<double>(values.size());

	std::vector<double> sortedValues = values;
	std::sort(sortedValues.begin(), sortedValues.end());

	auto getPercentile = [&sortedValues](double percentile) {
		double index = percentile * static_cast<double>(sortedValues.size() - 1);
		std::size_t lowerIndex = static_cast<std::size_t>(std::floor(index));
		std::size_t upperIndex = static_cast<std::size_t>(std::ceil(index));
		double mix = index - static_cast<double>(lowerIndex);

		return sortedValues[lowerIndex] + (sortedValues[upperIndex] - sortedValues[lowerIndex]) * mix;
	};

	summary.p95 = getPercentile(0.95);
	summary.p99 = getPercentile(0.99);

	return summary;
}

Profiler::Profiler(std::size_t historySize, std::size_t gpuFramesInFlight) {
	maxHistorySize = historySize;

	if (gpuFramesInFlight < 2) {
		gpuFramesInFlight = 2;
	}

	pendingFrames.resize(gpuFramesInFlight);
}

Profiler::~Profiler() {
	clearGpuState();

	for (GLuint queryId : freeQueries) {
		glDeleteQueries(1, &queryId);
	}
}

void Profiler::setEnabled(bool enabled) {
	this->enabled = enabled;

	if (!this->enabled) {
		frameActive = false;
		clearGpuState();
		drawCallCounter = 0;
		currentFrame = {};
	}
}

bool Profiler::isEnabled() const {
	return enabled;
}

void Profiler::beginFrame() {
	if (!enabled) {
		frameActive = false;
		return;
	}

	// We check OpenGL timer-query support only once
	if (!gpuSupportChecked) {
		gpuSupported = GLEW_VERSION_3_3 || GLEW_ARB_timer_query || GLEW_EXT_timer_query;
		gpuSupportChecked = true;
	}

	resolveReadyGpuFrames();

	frameCounter += 1;
	drawCallCounter = 0;

	currentFrame = {};
	currentFrame.frameIndex = frameCounter;
	currentFrame.gpuMs = std::numeric_limits<double>::quiet_NaN();
	currentSections.clear();
	gpuSectionOpen = false;
	frameActive = true;

	cpuFrameStart = Clock::now();
}

FrameStats Profiler::endFrame() {
	if (!enabled || !frameActive) {
		return FrameStats{};
	}

	currentFrame.cpuMs = std::chrono::duration<double, std::milli>(Clock::now() - cpuFrameStart).count();
	currentFrame.drawCalls = drawCallCounter;

	// If a section was left open, close it so the frame stays valid
	if (gpuSectionOpen) {
		endGpuSection();
	}

	// We do not ask OpenGL for GPU results right away
	// Instead, we store this frame's queries and read them in a later frame
	if (!currentSections.empty()) {
		PendingFrame& pendingFrame = pendingFrames[nextPendingFrameIndex];

		if (!pendingFrame.pending) {
			pendingFrame.pending = true;
			pendingFrame.frameIndex = currentFrame.frameIndex;
			pendingFrame.sections = currentSections;
			currentSections.clear();

			nextPendingFrameIndex += 1;
			if (nextPendingFrameIndex >= pendingFrames.size()) {
				nextPendingFrameIndex = 0;
			}
		}
		else {
			// If all pending slots are still busy, we drop current GPU timings instead of stalling the application
			for (GpuSection& section : currentSections) {
				storeQuery(section.queryId);
				section.queryId = 0;
			}

			currentSections.clear();
			gpuSectionOpen = false;
		}
	}

	if (maxHistorySize > 0 && history.size() == maxHistorySize) {
		history.pop_front();
	}
	history.push_back(currentFrame);

	resolveReadyGpuFrames();
	frameActive = false;

	return history.back();
}

void Profiler::beginGpuSection(const std::string& name) {
	if (!enabled || !gpuSupported || name.empty()) {
		return;
	}

	if (gpuSectionOpen) {
		endGpuSection();
	}

	GpuSection section;
	section.name = name;
	section.queryId = getQuery();

	glBeginQuery(GL_TIME_ELAPSED, section.queryId);
	currentSections.push_back(section);
	gpuSectionOpen = true;
}

void Profiler::endGpuSection() {
	if (!enabled || !gpuSupported || !gpuSectionOpen) {
		return;
	}

	glEndQuery(GL_TIME_ELAPSED);
	gpuSectionOpen = false;
}

void Profiler::incrementDrawCall(std::uint32_t count) {
	if (!enabled) {
		return;
	}

	drawCallCounter += count;
}

const FrameStats* Profiler::getLatestFrame() const {
	if (history.empty()) {
		return nullptr;
	}

	return &history.back();
}

const std::deque<FrameStats>& Profiler::getHistory() const {
	return history;
}

MetricSummary Profiler::getCpuSummary() const {
	std::vector<double> values;
	values.reserve(history.size());

	for (const FrameStats& frame : history) {
		values.push_back(frame.cpuMs);
	}

	return buildSummaryFromValues(values);
}

MetricSummary Profiler::getGpuSummary() const {
	std::vector<double> values;
	values.reserve(history.size());

	for (const FrameStats& frame : history) {
		if (std::isfinite(frame.gpuMs)) {
			values.push_back(frame.gpuMs);
		}
	}

	return buildSummaryFromValues(values);
}

MetricSummary Profiler::getDrawCallSummary() const {
	std::vector<double> values;
	values.reserve(history.size());

	for (const FrameStats& frame : history) {
		values.push_back(static_cast<double>(frame.drawCalls));
	}

	return buildSummaryFromValues(values);
}

MetricSummary Profiler::getGpuSectionSummary(const std::string& sectionName) const {
	std::vector<double> values;
	values.reserve(history.size());

	for (const FrameStats& frame : history) {
		auto it = frame.gpuSectionsMs.find(sectionName);
		if (it != frame.gpuSectionsMs.end()) {
			values.push_back(it->second);
		}
	}

	return buildSummaryFromValues(values);
}

bool Profiler::exportCsv(const std::string& filePath) const {
	std::ofstream file(filePath, std::ios::out | std::ios::trunc);
	if (!file) {
		return false;
	}

	std::vector<std::string> sectionNames;
	std::unordered_map<std::string, bool> sectionSeen;

	for (const FrameStats& frame : history) {
		for (const auto& pair : frame.gpuSectionsMs) {
			if (!sectionSeen[pair.first]) {
				sectionSeen[pair.first] = true;
				sectionNames.push_back(pair.first);
			}
		}
	}

	std::sort(sectionNames.begin(), sectionNames.end());

	file << "frame,cpu_ms,gpu_ms,draw_calls";
	file << ",cpu_avg_ms,cpu_p95_ms,cpu_p99_ms";
	file << ",gpu_avg_ms,gpu_p95_ms,gpu_p99_ms";
	for (const std::string& sectionName : sectionNames) {
		file << "," << escapeCsv(sectionName + "_gpu_ms");
	}
	file << "\n";

	file << std::fixed << std::setprecision(3);

	std::vector<double> cpuValues;
	std::vector<double> gpuValues;

	for (const FrameStats& frame : history) {
		cpuValues.push_back(frame.cpuMs);
		MetricSummary cpuSummary = buildSummaryFromValues(cpuValues);

		MetricSummary gpuSummary{};
		if (std::isfinite(frame.gpuMs)) {
			gpuValues.push_back(frame.gpuMs);
		}
		gpuSummary = buildSummaryFromValues(gpuValues);

		file << frame.frameIndex << ",";
		file << frame.cpuMs << ",";

		if (std::isfinite(frame.gpuMs)) {
			file << frame.gpuMs;
		}

		file << "," << frame.drawCalls;
		file << "," << cpuSummary.average;
		file << "," << cpuSummary.p95;
		file << "," << cpuSummary.p99;
		file << "," << gpuSummary.average;
		file << "," << gpuSummary.p95;
		file << "," << gpuSummary.p99;

		for (const std::string& sectionName : sectionNames) {
			file << ",";

			auto it = frame.gpuSectionsMs.find(sectionName);
			if (it != frame.gpuSectionsMs.end()) {
				file << it->second;
			}
		}

		file << "\n";
	}

	return true;
}

bool Profiler::isGpuTimingSupported() const {
	return gpuSupported;
}

Profiler::ScopedGpuSection::ScopedGpuSection(Profiler& profiler, const char* name) {
	this->profiler = &profiler;

	if (name != nullptr) {
		this->profiler->beginGpuSection(name);
	}
}

Profiler::ScopedGpuSection::~ScopedGpuSection() {
	if (profiler != nullptr) {
		profiler->endGpuSection();
	}
}

Profiler::ScopedBinding::ScopedBinding(Profiler& profiler) {
	previousProfiler = activeProfiler;
	activeProfiler = &profiler;
}

Profiler::ScopedBinding::~ScopedBinding() {
	activeProfiler = previousProfiler;
}

Profiler* Profiler::GetActive() {
	return activeProfiler;
}

void Profiler::clearGpuState() {
	if (gpuSectionOpen) {
		glEndQuery(GL_TIME_ELAPSED);
		gpuSectionOpen = false;
	}

	for (GpuSection& section : currentSections) {
		storeQuery(section.queryId);
		section.queryId = 0;
	}
	currentSections.clear();

	for (PendingFrame& frame : pendingFrames) {
		frame.pending = false;

		for (GpuSection& section : frame.sections) {
			storeQuery(section.queryId);
			section.queryId = 0;
		}

		frame.sections.clear();
	}
}

void Profiler::resolveReadyGpuFrames() {
	if (!gpuSupported) {
		return;
	}

	for (PendingFrame& frame : pendingFrames) {
		if (!frame.pending) {
			continue;
		}

		bool allQueriesReady = true;
		for (const GpuSection& section : frame.sections) {
			GLint available = GL_FALSE;
			glGetQueryObjectiv(section.queryId, GL_QUERY_RESULT_AVAILABLE, &available);

			if (available != GL_TRUE) {
				allQueriesReady = false;
				break;
			}
		}

		if (!allQueriesReady) {
			continue;
		}

		double totalGpuMs = 0.0;
		std::unordered_map<std::string, double> sectionTimes;

		for (GpuSection& section : frame.sections) {
			GLuint64 timeNs = 0;
			glGetQueryObjectui64v(section.queryId, GL_QUERY_RESULT, &timeNs);

			double timeMs = static_cast<double>(timeNs) * NANOSECONDS_TO_MILLISECONDS;
			totalGpuMs += timeMs;
			sectionTimes[section.name] += timeMs;

			storeQuery(section.queryId);
			section.queryId = 0;
		}

		for (FrameStats& historyFrame : history) {
			if (historyFrame.frameIndex == frame.frameIndex) {
				historyFrame.gpuMs = totalGpuMs;
				historyFrame.gpuSectionsMs = std::move(sectionTimes);
				break;
			}
		}

		frame.pending = false;
		frame.sections.clear();
	}
}

GLuint Profiler::getQuery() {
	if (!freeQueries.empty()) {
		GLuint queryId = freeQueries.back();
		freeQueries.pop_back();
		return queryId;
	}

	GLuint queryId = 0;
	glGenQueries(1, &queryId);
	return queryId;
}

void Profiler::storeQuery(GLuint queryId) {
	if (queryId != 0) {
		freeQueries.push_back(queryId);
	}
}

namespace ProfilerGL {
	void DrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {
		if (Profiler* profiler = Profiler::GetActive()) {
			profiler->incrementDrawCall();
		}

		glDrawElements(mode, count, type, indices);
	}

	void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instanceCount) {
		if (Profiler* profiler = Profiler::GetActive()) {
			profiler->incrementDrawCall();
		}

		glDrawElementsInstanced(mode, count, type, indices, instanceCount);
	}

	void DrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void* indices, GLint baseVertex) {
		if (Profiler* profiler = Profiler::GetActive()) {
			profiler->incrementDrawCall();
		}

		glDrawElementsBaseVertex(mode, count, type, const_cast<void*>(indices), baseVertex);
	}
}
