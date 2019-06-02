/////////////////////////////////////////////////////////////////////////////////////
///
/// @file
/// @author Kuba Sejdak
/// @copyright BSD 2-Clause License
///
/// Copyright (c) 2017-2019, Kuba Sejdak <kuba.sejdak@gmail.com>
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

#include <catch2/catch.hpp>

namespace Catch {

class VerboseReporter : public StreamingReporterBase<VerboseReporter> {
public:
    explicit VerboseReporter(const ReporterConfig& config)
        : StreamingReporterBase(config)
    {
        m_reporterPrefs.shouldRedirectStdOut = false;
    }

    static std::string getDescription()
    {
        return "Reports test results in a similar way to Google Test";
    }

private:
    void testCaseStarting(const TestCaseInfo& testInfo) override
    {
        m_testTimer.start();
        StreamingReporterBase::testCaseStarting(testInfo);

        stream << "Start TEST CASE '" << testInfo.name << "'\n";
        stream.flush();
    }

    void sectionStarting(const SectionInfo& sectionInfo) override
    {
        StreamingReporterBase::sectionStarting(sectionInfo);

        stream << "    Start SECTION '" << sectionInfo.name << "'\n";
        stream.flush();
    }

    void assertionStarting(const AssertionInfo&) override {}

    bool assertionEnded(const AssertionStats& assertionStats) override
    {
        const AssertionResult& result = assertionStats.assertionResult;
        if (result.isOk())
            return true;

        stream << "    [\n";
        stream << "        Failed test    : " << currentTestCaseInfo->name << "\n";
        stream << "        Failed line    : " << result.getSourceInfo() << "\n";
        stream << "        Type           : ";
        switch (result.getResultType()) {
            case ResultWas::ExpressionFailed:
                stream << "Expression failed";
                break;
            case ResultWas::ThrewException:
                stream << "Unexpected exception";
                break;
            case ResultWas::FatalErrorCondition:
                stream << "Fatal error condition";
                break;
            case ResultWas::DidntThrowException:
                stream << "No exception was thrown where one was expected";
                break;
            case ResultWas::ExplicitFailure:
                stream << "Explicit failure";
                break;
            default:
                return true;
        }
        stream << "\n";

        if (!assertionStats.infoMessages.empty()) {
            stream << "        Message(s)     : " << assertionStats.infoMessages[0].message << "\n";
            for (std::size_t i = 1; i < assertionStats.infoMessages.size(); ++i)
                stream << "                         " << assertionStats.infoMessages[i].message << "\n";
        }

        if (result.hasExpression()) {
            stream << "        Expression     : " << result.getExpressionInMacro() << "\n";
            stream << "        With expansion : " << result.getExpandedExpression() << "\n";
        }

        stream << "    ]\n";

        stream.flush();
        return true;
    }

    void sectionEnded(const SectionStats& sectionStats) override
    {
        stream << "    ";

        const auto& assertions = sectionStats.assertions;
        if (assertions.allPassed()) {
            Colour color(Colour::Green);
            stream << "[OK]";
        }
        else {
            Colour color(Colour::Red);
            stream << "[FAIL]";
        }

        stream << " End SECTION '" << sectionStats.sectionInfo.name << "'\n";
    }

    void testCaseEnded(const TestCaseStats& testCaseStats) override
    {
        StreamingReporterBase::testCaseEnded(testCaseStats);

        const auto& assertions = testCaseStats.totals.assertions;
        if (assertions.allPassed()) {
            Colour color(Colour::Green);
            stream << "[OK]";
        }
        else {
            Colour color(Colour::Red);
            stream << "[FAIL]";
        }

        auto elapsedMs = m_testTimer.getElapsedMilliseconds();
        stream << " End TEST CASE [" <<  elapsedMs << " ms]\n";
    }

private:
    Timer m_testTimer;
};

CATCH_REGISTER_REPORTER("verbose", VerboseReporter) // NOLINT

} // end namespace Catch
