#include <Optimizer/tests/gtest_base_tpcds_plan_test.h>

#include <QueryPlan/PlanPrinter.h>
#include <Optimizer/Iterative/IterativeRewriter.h>
#include <gtest/gtest.h>

using namespace DB;
using namespace std::string_literals;

class PlanCheckTpcds1000NotShowStats : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        auto settings = BasePlanTest::getDefaultOptimizerSettings();
        tester = std::make_shared<DB::BaseTpcdsPlanTest>(settings);
        tester->setShowStatistics(false);
        tester->setLabel("not_show_stats");
    }

    static void TearDownTestSuite()
    {
        tester.reset();
    }

    static std::string explain(const std::string & name)
    {
        return tester->explain(name);
    }

    static std::string expected(const std::string & name)
    {
        return tester->loadExplain(name);
    }

    static testing::AssertionResult equals(const std::string & actual, const std::string & expected)
    {
        if (actual == expected)
            return testing::AssertionSuccess();
        else
            return testing::AssertionFailure() << "\nexpected:\n" << expected << "\nactual:\n" << actual;
    }


    static std::shared_ptr<DB::BaseTpcdsPlanTest> tester;
};

std::shared_ptr<DB::BaseTpcdsPlanTest> PlanCheckTpcds1000NotShowStats::tester;

DECLARE_GENERATE_TEST(PlanCheckTpcds1000NotShowStats)

TEST_F(PlanCheckTpcds1000NotShowStats, q1)
{
    EXPECT_TRUE(equals(explain("q1"), expected("q1")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q2)
{
    EXPECT_TRUE(equals(explain("q2"), expected("q2")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q3)
{
    EXPECT_TRUE(equals(explain("q3"), expected("q3")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q4)
{
    EXPECT_TRUE(equals(explain("q4"), expected("q4")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q5)
{
    EXPECT_TRUE(equals(explain("q5"), expected("q5")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q6)
{
    EXPECT_TRUE(equals(explain("q6"), expected("q6")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q7)
{
    EXPECT_TRUE(equals(explain("q7"), expected("q7")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q8)
{
    EXPECT_TRUE(equals(explain("q8"), expected("q8")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q9)
{
    EXPECT_TRUE(equals(explain("q9"), expected("q9")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q10)
{
    EXPECT_TRUE(equals(explain("q10"), expected("q10")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q11)
{
    EXPECT_TRUE(equals(explain("q11"), expected("q11")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q12)
{
    EXPECT_TRUE(equals(explain("q12"), expected("q12")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q13)
{
    EXPECT_TRUE(equals(explain("q13"), expected("q13")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q14)
{
    EXPECT_TRUE(equals(explain("q14"), expected("q14")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q15)
{
    EXPECT_TRUE(equals(explain("q15"), expected("q15")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q16)
{
    EXPECT_TRUE(equals(explain("q16"), expected("q16")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q17)
{
    EXPECT_TRUE(equals(explain("q17"), expected("q17")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q18)
{
    EXPECT_TRUE(equals(explain("q18"), expected("q18")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q19)
{
    EXPECT_TRUE(equals(explain("q19"), expected("q19")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q20)
{
    EXPECT_TRUE(equals(explain("q20"), expected("q20")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q21)
{
    EXPECT_TRUE(equals(explain("q21"), expected("q21")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q22)
{
    EXPECT_TRUE(equals(explain("q22"), expected("q22")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q23)
{
    EXPECT_TRUE(equals(explain("q23"), expected("q23")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q24)
{
    EXPECT_TRUE(equals(explain("q24"), expected("q24")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q25)
{
    EXPECT_TRUE(equals(explain("q25"), expected("q25")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q26)
{
    EXPECT_TRUE(equals(explain("q26"), expected("q26")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q27)
{
    EXPECT_TRUE(equals(explain("q27"), expected("q27")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q28)
{
    EXPECT_TRUE(equals(explain("q28"), expected("q28")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q29)
{
    EXPECT_TRUE(equals(explain("q29"), expected("q29")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q30)
{
    EXPECT_TRUE(equals(explain("q30"), expected("q30")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q31)
{
    EXPECT_TRUE(equals(explain("q31"), expected("q31")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q32)
{
    EXPECT_TRUE(equals(explain("q32"), expected("q32")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q33)
{
    EXPECT_TRUE(equals(explain("q33"), expected("q33")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q34)
{
    EXPECT_TRUE(equals(explain("q34"), expected("q34")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q35)
{
    EXPECT_TRUE(equals(explain("q35"), expected("q35")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q36)
{
    EXPECT_TRUE(equals(explain("q36"), expected("q36")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q37)
{
    EXPECT_TRUE(equals(explain("q37"), expected("q37")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q38)
{
    EXPECT_TRUE(equals(explain("q38"), expected("q38")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q39)
{
    EXPECT_TRUE(equals(explain("q39"), expected("q39")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q40)
{
    EXPECT_TRUE(equals(explain("q40"), expected("q40")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q41)
{
    EXPECT_TRUE(equals(explain("q41"), expected("q41")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q42)
{
    EXPECT_TRUE(equals(explain("q42"), expected("q42")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q43)
{
    EXPECT_TRUE(equals(explain("q43"), expected("q43")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q44)
{
    EXPECT_TRUE(equals(explain("q44"), expected("q44")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q45)
{
    EXPECT_TRUE(equals(explain("q45"), expected("q45")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q46)
{
    EXPECT_TRUE(equals(explain("q46"), expected("q46")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q47)
{
    EXPECT_TRUE(equals(explain("q47"), expected("q47")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q48)
{
    EXPECT_TRUE(equals(explain("q48"), expected("q48")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q49)
{
    EXPECT_TRUE(equals(explain("q49"), expected("q49")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q50)
{
    EXPECT_TRUE(equals(explain("q50"), expected("q50")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q51)
{
    EXPECT_TRUE(equals(explain("q51"), expected("q51")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q52)
{
    EXPECT_TRUE(equals(explain("q52"), expected("q52")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q53)
{
    EXPECT_TRUE(equals(explain("q53"), expected("q53")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q54)
{
    EXPECT_TRUE(equals(explain("q54"), expected("q54")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q55)
{
    EXPECT_TRUE(equals(explain("q55"), expected("q55")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q56)
{
    EXPECT_TRUE(equals(explain("q56"), expected("q56")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q57)
{
    EXPECT_TRUE(equals(explain("q57"), expected("q57")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q58)
{
    EXPECT_TRUE(equals(explain("q58"), expected("q58")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q59)
{
    EXPECT_TRUE(equals(explain("q59"), expected("q59")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q60)
{
    EXPECT_TRUE(equals(explain("q60"), expected("q60")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q61)
{
    EXPECT_TRUE(equals(explain("q61"), expected("q61")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q62)
{
    EXPECT_TRUE(equals(explain("q62"), expected("q62")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q63)
{
    EXPECT_TRUE(equals(explain("q63"), expected("q63")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q64)
{
    EXPECT_TRUE(equals(explain("q64"), expected("q64")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q65)
{
    EXPECT_TRUE(equals(explain("q65"), expected("q65")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q66)
{
    EXPECT_TRUE(equals(explain("q66"), expected("q66")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q67)
{
    EXPECT_TRUE(equals(explain("q67"), expected("q67")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q68)
{
    EXPECT_TRUE(equals(explain("q68"), expected("q68")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q69)
{
    EXPECT_TRUE(equals(explain("q69"), expected("q69")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q70)
{
    EXPECT_TRUE(equals(explain("q70"), expected("q70")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q71)
{
    EXPECT_TRUE(equals(explain("q71"), expected("q71")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q72)
{
    EXPECT_TRUE(equals(explain("q72"), expected("q72")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q73)
{
    EXPECT_TRUE(equals(explain("q73"), expected("q73")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q74)
{
    EXPECT_TRUE(equals(explain("q74"), expected("q74")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q75)
{
    EXPECT_TRUE(equals(explain("q75"), expected("q75")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q76)
{
    EXPECT_TRUE(equals(explain("q76"), expected("q76")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q77)
{
    EXPECT_TRUE(equals(explain("q77"), expected("q77")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q78)
{
    EXPECT_TRUE(equals(explain("q78"), expected("q78")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q79)
{
    EXPECT_TRUE(equals(explain("q79"), expected("q79")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q80)
{
    EXPECT_TRUE(equals(explain("q80"), expected("q80")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q81)
{
    EXPECT_TRUE(equals(explain("q81"), expected("q81")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q82)
{
    EXPECT_TRUE(equals(explain("q82"), expected("q82")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q83)
{
    EXPECT_TRUE(equals(explain("q83"), expected("q83")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q84)
{
    EXPECT_TRUE(equals(explain("q84"), expected("q84")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q85)
{
    EXPECT_TRUE(equals(explain("q85"), expected("q85")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q86)
{
    EXPECT_TRUE(equals(explain("q86"), expected("q86")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q87)
{
    EXPECT_TRUE(equals(explain("q87"), expected("q87")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q88)
{
    EXPECT_TRUE(equals(explain("q88"), expected("q88")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q89)
{
    EXPECT_TRUE(equals(explain("q89"), expected("q89")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q90)
{
    EXPECT_TRUE(equals(explain("q90"), expected("q90")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q91)
{
    EXPECT_TRUE(equals(explain("q91"), expected("q91")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q92)
{
    EXPECT_TRUE(equals(explain("q92"), expected("q92")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q93)
{
    EXPECT_TRUE(equals(explain("q93"), expected("q93")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q94)
{
    EXPECT_TRUE(equals(explain("q94"), expected("q94")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q95)
{
    EXPECT_TRUE(equals(explain("q95"), expected("q95")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q96)
{
    EXPECT_TRUE(equals(explain("q96"), expected("q96")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q97)
{
    EXPECT_TRUE(equals(explain("q97"), expected("q97")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q98)
{
    EXPECT_TRUE(equals(explain("q98"), expected("q98")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, q99)
{
    EXPECT_TRUE(equals(explain("q99"), expected("q99")));
}

TEST_F(PlanCheckTpcds1000NotShowStats, summary)
{
    std::cout << "rule call times:" << std::endl;
    for (const auto & x: IterativeRewriter::getRuleCallTimes())
        std::cout << x.first << ": " << x.second << std::endl;
}
