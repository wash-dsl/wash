#include "../src/wash/wash.hpp"
#include <gtest/gtest.h>

TEST(ParticleTest, TestParticleEucDist) {
    wash::Particle p1{0, 1.0, 0.35, 1.0, wash::Vec3D{1,-2,3}, wash::Vec3D{}, wash::Vec3D{}};
    wash::Particle p2{0, 1.0, 0.35, 1.0, wash::Vec3D{-3,2,1}, wash::Vec3D{}, wash::Vec3D{}};

    EXPECT_EQ(wash::eucdist(p1, p2), 6);
}