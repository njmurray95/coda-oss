/* =========================================================================
 * This file is part of logging-c++
 * =========================================================================
 *
 * (C) Copyright 2013, General Dynamics - Advanced Information Systems
 *
 * logging-c++ is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not,
 * see <http://www.gnu.org/licenses/>.
 *
 */

#include <import/logging.h>
#include <import/mt.h>
#include "TestCase.h"

class RunNothing : public mt::Runnable
{
private:
    size_t& counter;
    logging::ExceptionLogger* exLog;
    static mt::Mutex counterLock;
public:
    RunNothing(size_t& c, logging::ExceptionLogger* el) : counter(c), exLog(el) {}

    virtual void run()
    {
        if(exLog->hasLogged())
            return;
       
        {
            mt::CriticalSection<mt::Mutex> crit(&counterLock);
            counter++;
        }

        exLog->log(except::Exception("Bad run"), logging::LogLevel::LOG_ERROR);
    }
};

mt::Mutex RunNothing::counterLock;

TEST_CASE(testExceptionLogger)
{
    std::auto_ptr<logging::Logger> log(new logging::Logger("test"));
    std::auto_ptr<logging::Formatter>
            formatter(new logging::StandardFormatter("%m"));

    mem::SharedPtr<logging::ExceptionLogger> exLog(new logging::ExceptionLogger(log.get()));

    size_t counter(0);
    size_t numThreads(2);
 
    std::vector<mt::Runnable*> runs;
   
    mt::GenerationThreadPool pool(numThreads);
    pool.start();

    runs.push_back(new RunNothing(counter, exLog.get()));
    pool.addAndWaitGroup(runs);
    runs.clear();

    runs.push_back(new RunNothing(counter, exLog.get()));
    pool.addAndWaitGroup(runs);
    runs.clear();

    TEST_ASSERT(counter == 1);
}

int main(int argc, char* argv[])
{
    TEST_CHECK(testExceptionLogger);
}
