/**
 * @file test_ResonanceField.cpp
 * @brief Tests for ResonanceField gauge, votes, trigger, and Resonance Memory.
 */
#include "Battle/ResonanceField.h"
#include "Core/Affinity.h"
#include "doctest.h"
#include <ostream>
#include <string>

TEST_CASE("ResonanceField: gauge starts at 0")
{
    ResonanceField field{};
    CHECK(field.getGauge() == 0);
    CHECK(!field.isReady());
}

TEST_CASE("ResonanceField: addContribution increments gauge")
{
    ResonanceField field{};
    field.addContribution(Affinity::Blaze, 30);
    CHECK(field.getGauge() == 30);
    CHECK(!field.isReady());
}

TEST_CASE("ResonanceField: gauge clamps at kGaugeCap")
{
    ResonanceField field{};
    field.addContribution(Affinity::Blaze, 60);
    field.addContribution(Affinity::Blaze, 60); // would exceed 100
    CHECK(field.getGauge() == ResonanceField::kGaugeCap);
}

TEST_CASE("ResonanceField: isReady when gauge >= 100")
{
    ResonanceField field{};
    field.addContribution(Affinity::Frost, 100);
    CHECK(field.isReady());
}

TEST_CASE("ResonanceField: normal affinity adds 1 vote")
{
    ResonanceField field{};
    field.addContribution(Affinity::Blaze, 10);
    field.addContribution(Affinity::Blaze, 10);
    CHECK(field.getVotes(Affinity::Blaze) == doctest::Approx(2.0f));
    CHECK(field.getVotes(Affinity::Frost) == doctest::Approx(0.0f));
}

TEST_CASE("ResonanceField: Aether adds 0.5 votes to every affinity")
{
    ResonanceField field{};
    field.addContribution(Affinity::Aether, 10);
    CHECK(field.getVotes(Affinity::Blaze) == doctest::Approx(0.5f));
    CHECK(field.getVotes(Affinity::Frost) == doctest::Approx(0.5f));
    CHECK(field.getVotes(Affinity::Tempest) == doctest::Approx(0.5f));
    CHECK(field.getVotes(Affinity::Terra) == doctest::Approx(0.5f));
    CHECK(field.getVotes(Affinity::Aether) == doctest::Approx(0.5f));
}

TEST_CASE("ResonanceField: trigger returns dominant affinity")
{
    ResonanceField field{};
    field.addContribution(Affinity::Blaze, 50);
    field.addContribution(Affinity::Blaze, 50);
    // Two Blaze contributions = 2 votes
    field.addContribution(Affinity::Frost, 10);
    // One Frost contribution = 1 vote

    Affinity winner = field.trigger();
    CHECK(winner == Affinity::Blaze);
}

TEST_CASE("ResonanceField: trigger resets gauge to 0")
{
    ResonanceField field{};
    field.addContribution(Affinity::Blaze, 100);
    field.trigger();
    CHECK(field.getGauge() == 0);
}

TEST_CASE("ResonanceField: Resonance Memory retains 25% of winning votes")
{
    ResonanceField field{};
    // 4 Blaze contributions = 4.0 votes; 1 Frost = 1.0 vote
    field.addContribution(Affinity::Blaze, 25);
    field.addContribution(Affinity::Blaze, 25);
    field.addContribution(Affinity::Blaze, 25);
    field.addContribution(Affinity::Blaze, 25);
    field.addContribution(Affinity::Frost, 10);

    field.trigger(); // Blaze wins

    // 25% of 4.0 = 1.0 Blaze votes retained; Frost resets to 0.
    CHECK(field.getVotes(Affinity::Blaze) == doctest::Approx(1.0f));
    CHECK(field.getVotes(Affinity::Frost) == doctest::Approx(0.0f));
}

TEST_CASE("ResonanceField: tie between affinities resolved by lower enum value")
{
    ResonanceField field{};
    // Both Blaze and Frost have 1 vote; Blaze has lower enum value.
    field.addContribution(Affinity::Blaze, 50);
    field.addContribution(Affinity::Frost, 50);
    Affinity winner = field.trigger();
    CHECK(winner == Affinity::Blaze);
}

TEST_CASE("ResonanceField: reset clears gauge and all votes")
{
    ResonanceField field{};
    field.addContribution(Affinity::Terra, 50);
    field.reset();
    CHECK(field.getGauge() == 0);
    CHECK(field.getVotes(Affinity::Terra) == doctest::Approx(0.0f));
}

TEST_CASE("ResonanceField: getVoteSummary lists only affinities with votes")
{
    ResonanceField field{};
    field.addContribution(Affinity::Blaze, 10);
    field.addContribution(Affinity::Frost, 10);
    std::string summary = field.getVoteSummary();
    CHECK(summary.find("Blaze") != std::string::npos);
    CHECK(summary.find("Frost") != std::string::npos);
    CHECK(summary.find("Terra") == std::string::npos);
}