#include <Preferences.h>
#include "Settings.h"
#include "config.h"

namespace {

Preferences prefs;
constexpr char kNamespace[] = "settings";
constexpr char kJogStepsKey[] = "jogSteps";
constexpr char kAutoFireKey[] = "autoFire";

}  // namespace

void Settings::begin() { prefs.begin(kNamespace, /*readOnly=*/false); }

long Settings::jogSteps() const { return prefs.getLong(kJogStepsKey, DEFAULT_JOG_STEPS); }

void Settings::setJogSteps(long steps) { prefs.putLong(kJogStepsKey, steps); }

bool Settings::autoFireOnAdvance() const {
  return prefs.getBool(kAutoFireKey, DEFAULT_AUTO_FIRE_ON_ADVANCE);
}

void Settings::setAutoFireOnAdvance(bool enabled) { prefs.putBool(kAutoFireKey, enabled); }
