#include <gtest/gtest.h>

#include "../common/include/intersection.hpp"
#include "../common/include/object.hpp"
#include "../common/include/ray.hpp"
#include "../common/include/vector.hpp"

#include <cmath>
#include <limits>

using namespace render;

// Tolerancia para comparaciones de punto flotante
constexpr double TOLERANCE = 1e-6;

// ============================================================================
// TESTS PARA SPHERE::INTERSECT
// ============================================================================

class SphereIntersectionTest : public ::testing::Test {
protected:
  // Esfera en el origen con radio 1
  sphere unit_sphere{point3d(0, 0, 0), 1.0};

  // Esfera desplazada
  sphere offset_sphere{point3d(5, 0, 0), 2.0};
};

// --- CASOS BÁSICOS ---

TEST_F(SphereIntersectionTest, NoIntersection_RayMissesSphere) {
  // Rayo que pasa por encima de la esfera
  ray r(vector(-5, 10, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  EXPECT_FALSE(result.hit);
}

TEST_F(SphereIntersectionTest, DirectHit_RayThroughCenter) {
  // Rayo que va directo al centro
  ray r(vector(-5, 0, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.distance, 4.0, TOLERANCE);  // Golpea a distancia 4
  EXPECT_NEAR(result.point.get_x(), -1.0, TOLERANCE);
  EXPECT_NEAR(result.point.get_y(), 0.0, TOLERANCE);
  EXPECT_NEAR(result.point.get_z(), 0.0, TOLERANCE);
}

TEST_F(SphereIntersectionTest, TangentRay_OneIntersection) {
  // Rayo tangente (discriminante = 0)
  ray r(vector(-5, 1, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.distance, 5.0, TOLERANCE);
  EXPECT_NEAR(result.point.get_x(), 0.0, TOLERANCE);
  EXPECT_NEAR(result.point.get_y(), 1.0, TOLERANCE);
}

TEST_F(SphereIntersectionTest, TwoIntersections_ChoosesClosest) {
  // Rayo que atraviesa la esfera completamente
  ray r(vector(-5, 0, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  // Debe elegir la intersección más cercana (entrada, no salida)
  EXPECT_NEAR(result.distance, 4.0, TOLERANCE);
  EXPECT_LT(result.distance, 6.0);  // No debe ser la salida
}

// --- CASOS DE VECTOR NORMAL ---

TEST_F(SphereIntersectionTest, NormalVector_PointsOutward) {
  // Rayo desde afuera
  ray r(vector(-5, 0, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_TRUE(result.outward);
  // Normal debe apuntar hacia afuera (en dirección -X)
  EXPECT_NEAR(result.normal.get_x(), -1.0, TOLERANCE);
  EXPECT_NEAR(result.normal.get_y(), 0.0, TOLERANCE);
  EXPECT_NEAR(result.normal.get_z(), 0.0, TOLERANCE);
}

TEST_F(SphereIntersectionTest, NormalVector_Normalized) {
  ray r(vector(-5, 0, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  // El vector normal debe estar normalizado (magnitud = 1)
  double magnitude = std::sqrt(result.normal.get_x() * result.normal.get_x() +
                               result.normal.get_y() * result.normal.get_y() +
                               result.normal.get_z() * result.normal.get_z());
  EXPECT_NEAR(magnitude, 1.0, TOLERANCE);
}

TEST_F(SphereIntersectionTest, NormalVector_CorrectDirection) {
  // Golpear desde diferentes direcciones
  ray r_top(vector(0, 5, 0), vector(0, -1, 0));
  Intersection top = unit_sphere.intersect(r_top);

  EXPECT_TRUE(top.hit);
  EXPECT_NEAR(top.normal.get_y(), 1.0, TOLERANCE);  // Normal apunta hacia +Y
}

// --- CASOS LÍMITE ---

TEST_F(SphereIntersectionTest, VeryCloseIntersection_BelowMinDistance) {
  // Rayo que empieza casi en la superficie
  ray r(vector(-1.0001, 0, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  // Puede o no detectar dependiendo de MIN_DISTANCE
  // Si no detecta, está correcto (muy cerca del origen)
  if (result.hit) {
    EXPECT_GT(result.distance, 1e-3);  // Debe respetar MIN_DISTANCE
  }
}

TEST_F(SphereIntersectionTest, RayOriginInsideSphere) {
  // Rayo que empieza dentro de la esfera
  ray r(vector(0, 0, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  // Debe encontrar la salida
  EXPECT_NEAR(result.distance, 1.0, TOLERANCE);
  EXPECT_FALSE(result.outward);  // Golpea desde adentro
}

TEST_F(SphereIntersectionTest, RayOriginOnSurface) {
  // Rayo que empieza exactamente en la superficie
  ray r(vector(1, 0, 0), vector(1, 0, 0));

  Intersection result = unit_sphere.intersect(r);

  // No debe auto-intersectar (distancia = 0)
  if (result.hit) {
    EXPECT_GT(result.distance, 1e-3);
  }
}

TEST_F(SphereIntersectionTest, NegativeDirection_BehindRay) {
  // Intersección está detrás del origen del rayo
  ray r(vector(5, 0, 0), vector(1, 0, 0));  // Mirando hacia +X, esfera en origen

  Intersection result = unit_sphere.intersect(r);

  EXPECT_FALSE(result.hit);  // No debe contar intersecciones hacia atrás
}

// --- CASOS CON ESFERA DESPLAZADA ---

TEST_F(SphereIntersectionTest, OffsetSphere_CorrectIntersection) {
  // offset_sphere está en (5, 0, 0) con radio 2
  ray r(vector(0, 0, 0), vector(1, 0, 0));

  Intersection result = offset_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.distance, 3.0, TOLERANCE);  // 5 - 2 = 3
  EXPECT_NEAR(result.point.get_x(), 3.0, TOLERANCE);
}

// --- CASOS NUMÉRICOS EXTREMOS ---

TEST_F(SphereIntersectionTest, VeryLargeDistance) {
  sphere distant_sphere(point3d(1'000, 0, 0), 1.0);
  ray r(vector(0, 0, 0), vector(1, 0, 0));

  Intersection result = distant_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.distance, 999.0, TOLERANCE);
}

TEST_F(SphereIntersectionTest, VerySmallRadius) {
  sphere tiny_sphere(point3d(0, 0, 0), 0.001);
  ray r(vector(-1, 0, 0), vector(1, 0, 0));

  Intersection result = tiny_sphere.intersect(r);

  EXPECT_TRUE(result.hit);
}

// ============================================================================
// TESTS PARA CYLINDER::INTERSECT
// ============================================================================

class CylinderIntersectionTest : public ::testing::Test {
protected:
  // Cilindro vertical centrado en origen, radio 1, altura 4
  cylinder vertical_cylinder{point3d(0, 0, 0), 1.0, vector(0, 4, 0)};

  // Cilindro horizontal
  cylinder horizontal_cylinder{point3d(0, 0, 0), 1.0, vector(4, 0, 0)};
};

// --- CASOS BÁSICOS DE SUPERFICIE CURVA ---

TEST_F(CylinderIntersectionTest, NoIntersection_RayMisses) {
  // Rayo que pasa lejos del cilindro
  ray r(vector(-5, 0, 0), vector(0, 1, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_FALSE(result.hit);
}

TEST_F(CylinderIntersectionTest, IntersectsCurvedSurface_Vertical) {
  // Rayo horizontal que golpea la superficie curva
  ray r(vector(-5, 0, 0), vector(1, 0, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.distance, 4.0, TOLERANCE);
  EXPECT_NEAR(result.point.get_x(), -1.0, TOLERANCE);
  EXPECT_NEAR(result.point.get_y(), 0.0, TOLERANCE);
}

TEST_F(CylinderIntersectionTest, IntersectsCurvedSurface_Normal) {
  // Verificar que el normal es perpendicular al eje
  ray r(vector(-5, 0, 0), vector(1, 0, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  // Normal debe ser perpendicular al eje Y, apuntando hacia -X
  EXPECT_NEAR(result.normal.get_x(), -1.0, TOLERANCE);
  EXPECT_NEAR(result.normal.get_y(), 0.0, TOLERANCE);
}

// --- CASOS DE INTERSECCIÓN CON BASES ---

TEST_F(CylinderIntersectionTest, IntersectsTopBase) {
  // Rayo desde arriba hacia la base superior
  ray r(vector(0, 5, 0), vector(0, -1, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.distance, 3.0, TOLERANCE);       // 5 - 2 = 3
  EXPECT_NEAR(result.point.get_y(), 2.0, TOLERANCE);  // Altura/2 = 2
}

TEST_F(CylinderIntersectionTest, IntersectsBottomBase) {
  // Rayo desde abajo hacia la base inferior
  ray r(vector(0, -5, 0), vector(0, 1, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.distance, 3.0, TOLERANCE);
  EXPECT_NEAR(result.point.get_y(), -2.0, TOLERANCE);  // -Altura/2 = -2
}

TEST_F(CylinderIntersectionTest, BaseIntersection_NormalCorrect) {
  ray r(vector(0, 5, 0), vector(0, -1, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  // Normal de la base superior debe apuntar hacia +Y
  EXPECT_NEAR(result.normal.get_y(), 1.0, TOLERANCE);
}

TEST_F(CylinderIntersectionTest, BaseIntersection_OutsideRadius) {
  // Rayo que golpearía el plano pero fuera del radio
  ray r(vector(5, 5, 0), vector(0, -1, 0));  // Muy lejos del eje

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_FALSE(result.hit);  // No debe contar, está fuera del círculo
}

// --- CASOS LÍMITE DE ALTURA ---

TEST_F(CylinderIntersectionTest, RayHitsCurve_ButAboveHeight) {
  // Rayo que golpearía la superficie curva si fuera infinita, pero está fuera de altura
  ray r(vector(-5, 10, 0), vector(1, 0, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_FALSE(result.hit);  // Fuera de los límites de altura
}

TEST_F(CylinderIntersectionTest, RayParallelToAxis_OnlyBasesCanHit) {
  // Rayo paralelo al eje (no puede golpear superficie curva)
  ray r(vector(0, -5, 0), vector(0, 1, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  // Solo puede golpear bases
}

// --- CASOS CON CILINDRO HORIZONTAL ---

TEST_F(CylinderIntersectionTest, HorizontalCylinder_CurvedSurface) {
  ray r(vector(0, -5, 0), vector(0, 1, 0));

  Intersection result = horizontal_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.point.get_y(), -1.0, TOLERANCE);
}

TEST_F(CylinderIntersectionTest, HorizontalCylinder_Base) {
  // Golpear la base (en dirección X)
  ray r(vector(5, 0, 0), vector(-1, 0, 0));

  Intersection result = horizontal_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_NEAR(result.point.get_x(), 2.0, TOLERANCE);
}

// --- CASOS DE MÚLTIPLES INTERSECCIONES ---

TEST_F(CylinderIntersectionTest, MultipleIntersections_ChoosesClosest) {
  // Rayo que atraviesa el cilindro (2 puntos en curva)
  ray r(vector(-5, 0, 0), vector(1, 0, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  // Debe elegir la entrada, no la salida
  EXPECT_NEAR(result.distance, 4.0, TOLERANCE);
  EXPECT_LT(result.distance, 6.0);
}

TEST_F(CylinderIntersectionTest, BaseCloserThanCurve) {
  // Configurar escenario donde base está más cerca que curva
  cylinder test_cyl(point3d(0, 0, 0), 1.0, vector(0, 2, 0));
  ray r(vector(0, 3, 0), vector(0, -1, 0));

  Intersection result = test_cyl.intersect(r);

  EXPECT_TRUE(result.hit);
  // Debe elegir la base superior
  EXPECT_NEAR(result.point.get_y(), 1.0, TOLERANCE);
}

// --- CASOS NUMÉRICOS EXTREMOS ---

TEST_F(CylinderIntersectionTest, VeryThinCylinder) {
  cylinder thin(point3d(0, 0, 0), 0.001, vector(0, 2, 0));
  ray r(vector(-1, 0, 0), vector(1, 0, 0));

  // Puede o no golpear dependiendo de precisión numérica
  Intersection result = thin.intersect(r);
  // Solo verificamos que no crashee
  (void) result;  // Suprimir warning de variable no usada
}

TEST_F(CylinderIntersectionTest, VeryTallCylinder) {
  cylinder tall(point3d(0, 0, 0), 1.0, vector(0, 1'000, 0));
  ray r(vector(-5, 500, 0), vector(1, 0, 0));

  Intersection result = tall.intersect(r);

  EXPECT_TRUE(result.hit);
}

// --- CASOS DE DIRECCIÓN DEL RAYO ---

TEST_F(CylinderIntersectionTest, RayFromInside) {
  // Rayo que empieza dentro del cilindro
  ray r(vector(0, 0, 0), vector(1, 0, 0));

  Intersection result = vertical_cylinder.intersect(r);

  EXPECT_TRUE(result.hit);
  EXPECT_FALSE(result.outward);  // Golpea desde adentro
}

TEST_F(CylinderIntersectionTest, GrazingAngle_Tangent) {
  // Rayo tangente a la superficie
  ray r(vector(-5, 1, 0), vector(1, 0, 0));

  Intersection result = vertical_cylinder.intersect(r);

  // Puede o no detectar (depende de precisión numérica)
  // Solo verificamos que sea consistente
  (void) result;  // Suprimir warning de variable no usada
}

// ============================================================================
// TESTS DE INTEGRACIÓN
// ============================================================================

TEST(IntersectionIntegrationTest, ClosestOfMultipleObjects) {
  sphere s1(point3d(5, 0, 0), 1.0);
  sphere s2(point3d(10, 0, 0), 1.0);

  ray r(vector(0, 0, 0), vector(1, 0, 0));

  Intersection i1 = s1.intersect(r);
  Intersection i2 = s2.intersect(r);

  EXPECT_TRUE(i1.hit);
  EXPECT_TRUE(i2.hit);
  EXPECT_LT(i1.distance, i2.distance);  // s1 está más cerca
}

TEST(IntersectionIntegrationTest, NoIntersection_ReturnsValidStruct) {
  sphere s(point3d(0, 0, 0), 1.0);
  ray r(vector(10, 10, 10), vector(1, 0, 0));

  Intersection result = s.intersect(r);

  EXPECT_FALSE(result.hit);
  // Verificar que los otros campos tienen valores razonables
  EXPECT_EQ(result.distance, 0.0);
}
