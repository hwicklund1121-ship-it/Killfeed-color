// Pure decision rules shared by the server checks and executable boundary tests.
class GRZAlertRules
{
 static bool ValidSample(int elapsed)
 {
  return elapsed >= 500 && elapsed <= 2500;
 }
 static bool FastMovement(float horizontalMetres, int elapsed, float limitMps)
 {
  return ValidSample(elapsed) && horizontalMetres * 1000.0 > limitMps * elapsed;
 }
 static bool FastCadence(int shots, int elapsed, float configuredMs, float toleranceRatio)
 {
  if (shots < 6 || elapsed < 2000 || configuredMs <= 0) return false;
  float meanInterval = elapsed;
  meanInterval = meanInterval / (shots - 1);
  return meanInterval < configuredMs * toleranceRatio;
 }
 static bool ExcessDamage(float observed, float configured, float envelope)
 {
  return configured > 0 && observed > configured * envelope;
 }
};
