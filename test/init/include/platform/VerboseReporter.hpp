/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2019-2023, Kuba Sejdak <kuba.sejdak@gmail.com>
/// All rights reserved.
///
/// Redistribution and use in source and binary forms, with or without
/// modification, are permitted provided that the following conditions are met:
///
/// 1. Redistributions of source code must retain the above copyright notice, this
///    list of conditions and the following disclaimer.
///
/// 2. Redistributions in binary form must reproduce the above copyright notice,
///    this list of conditions and the following disclaimer in the documentation
///    and/or other materials provided with the distribution.
///
/// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
/// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
/// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
/// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
/// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
/// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
/// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
/// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
/// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
/// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///
/////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <catch2/catch_assertion_info.hpp>
#include <catch2/catch_assertion_result.hpp>
#include <catch2/catch_test_case_info.hpp>
#include <catch2/catch_timer.hpp>
#include <catch2/interfaces/catch_interfaces_reporter.hpp>
#include <catch2/internal/catch_console_colour.hpp>
#include <catch2/internal/catch_istream.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <catch2/reporters/catch_reporter_streaming_base.hpp>

#include <cstddef>
#include <sstream>
#include <string>
#include <utility>

constexpr std::size_t cIndentationSize = 4;
constexpr std::size_t cStatusColumn = 100;
constexpr const char* cErrorSeparator = "====================";

namespace Catch {

class VerboseReporter : public StreamingReporterBase {
public:
    explicit VerboseReporter(ReporterConfig config)
        : StreamingReporterBase(std::move(config))
    {
        m_preferences.shouldRedirectStdOut = false;
    }

    static std::string getDescription() { return "Reports test results in a similar way to Google Test"; }

private:
    void testCaseStarting(const TestCaseInfo& testInfo) override
    {
        StreamingReporterBase::testCaseStarting(testInfo);
        m_testTimer.start();

        m_stream << "Start TEST CASE " << testInfo.name << "\n";
        m_stream.flush();
    }

    void sectionStarting(const SectionInfo& sectionInfo) override
    {
        StreamingReporterBase::sectionStarting(sectionInfo);
        m_lastSection = sectionInfo.name;

        increaseIndentation();

        m_stream << indentation() << "Start SECTION: " << sectionInfo.name << "\n";
        m_stream.flush();
    }

    void assertionEnded(const AssertionStats& assertionStats) override
    {
        const AssertionResult& result = assertionStats.assertionResult;
        if (result.isOk())
            return;

        m_stream << indentation() << cErrorSeparator << "\n";
        increaseIndentation();
        m_stream << indentation() << "Failed test    : " << m_sectionStack.back().name << "\n";
        m_stream << indentation() << "Last section   : " << m_lastSection << "\n";
        m_stream << indentation() << "Failed line    : " << result.getSourceInfo() << "\n";
        m_stream << indentation() << "Type           : ";
        switch (result.getResultType()) {
            case ResultWas::ExpressionFailed: m_stream << "Expression failed"; break;
            case ResultWas::ThrewException: m_stream << "Unexpected exception"; break;
            case ResultWas::FatalErrorCondition: m_stream << "Fatal error condition"; break;
            case ResultWas::DidntThrowException: m_stream << "No exception was thrown where one was expected"; break;
            case ResultWas::ExplicitFailure: m_stream << "Explicit failure"; break;
            default: return;
        }
        m_stream << "\n";

        if (!assertionStats.infoMessages.empty()) {
            m_stream << indentation() << "Message(s)     : " << assertionStats.infoMessages[0].message << "\n";
            for (std::size_t i = 1; i < assertionStats.infoMessages.size(); ++i)
                m_stream << "                         " << assertionStats.infoMessages[i].message << "\n";
        }

        if (result.hasExpression()) {
            m_stream << indentation() << "Expression     : " << result.getExpressionInMacro() << "\n";
            m_stream << indentation() << "With expansion : " << result.getExpandedExpression() << "\n";
        }

        decreaseIndentation();
        m_stream << indentation() << cErrorSeparator << "\n";
        m_stream.flush();
    }

    void sectionEnded(const SectionStats& sectionStats) override
    {
        std::stringstream ss;
        ss << indentation() << "End";
        m_stream << ss.str();
        m_stream.flush();
        printStatus(ss.str().size(), sectionStats.assertions.allOk());

        decreaseIndentation();

        StreamingReporterBase::sectionEnded(sectionStats);
    }

    void testCaseEnded(const TestCaseStats& testCaseStats) override
    {
        auto elapsedMs = m_testTimer.getElapsedMilliseconds();
        std::stringstream ss;
        ss << "End [" << elapsedMs << " ms]";
        m_stream << ss.str();
        printStatus(ss.str().size(), testCaseStats.totals.assertions.allOk());

        m_lastSection.clear();
        StreamingReporterBase::testCaseEnded(testCaseStats);
    }

    void testRunEnded(const TestRunStats& testRunStats) override
    {
        m_stream << "\n";
        m_stream << std::string(cStatusColumn, '=') << "\n";
        m_stream << "Finished TEST RUN : " << testRunStats.runInfo.name << "\n";
        m_stream << "Status            :";

        const auto& assertions = testRunStats.totals.assertions;
        printStatus(cStatusColumn, assertions.allOk(), true);
        m_stream << "Failed assertions : " << assertions.failed << " / " << assertions.total() << "\n";
        m_stream.flush();

        StreamingReporterBase::testRunEnded(testRunStats);
    }

    [[nodiscard]] std::string indentation() const
    {
        std::string indent(m_indentation, ' ');
        return indent;
    }

    void increaseIndentation() { m_indentation += cIndentationSize; }

    void decreaseIndentation() { m_indentation -= cIndentationSize; }

    void printStatus(std::size_t offset, bool success, bool finalStatus = false)
    {
        auto paddingSize = (cStatusColumn > offset) ? (cStatusColumn - offset) : 1;
        m_stream << std::string(paddingSize, ' ');

        if (success) {
            auto colorGuard = m_colour->guardColour(Colour::Green);
            colorGuard.engage(m_stream);
            m_stream << (finalStatus ? "[PASSED]" : "[OK]");
        }
        else {
            auto colorGuard = m_colour->guardColour(Colour::Red);
            colorGuard.engage(m_stream);
            m_stream << "[FAILED]";
        }

        m_stream << "\n";
        m_stream.flush();
    }

private:
    Timer m_testTimer;
    std::size_t m_indentation{};
    std::string m_lastSection;
};

CATCH_REGISTER_REPORTER("verbose", VerboseReporter) // NOLINT

} // end namespace Catch
