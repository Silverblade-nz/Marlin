/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2023 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

/**
 * lcd/extui/anycubic_vyper/vyper_extui.cpp
 *
 * Anycubic Dgus TFT support for Marlin
 */

#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(ANYCUBIC_LCD_VYPER)

#include "../ui_api.h"
#include "dgus_tft.h"

using namespace Anycubic;

namespace ExtUI {

  void onStartup() { dgus.startup(); }

  void onIdle() { dgus.idleLoop(); }

  void onPrinterKilled(FSTR_P const error, FSTR_P const component) {
    dgus.printerKilled(error, component);
  }

  void onMediaMounted() { dgus.mediaEvent(AC_media_inserted); }
  void onMediaError()   { dgus.mediaEvent(AC_media_error);    }
  void onMediaRemoved() { dgus.mediaEvent(AC_media_removed);  }

  void onHeatingError(const heater_id_t header_id) {}
  void onMinTempError(const heater_id_t header_id) {}
  void onMaxTempError(const heater_id_t header_id) {}

  void onPlayTone(const uint16_t frequency, const uint16_t duration/*=0*/) {
    #if ENABLED(SPEAKER)
      ::tone(BEEPER_PIN, frequency, duration);
    #endif
  }

  void onPrintTimerStarted() { dgus.timerEvent(AC_timer_started); }
  void onPrintTimerPaused()  { dgus.timerEvent(AC_timer_paused);  }
  void onPrintTimerStopped() { dgus.timerEvent(AC_timer_stopped); }

  void onPrintDone() {}

  void onFilamentRunout(const extruder_t)            { dgus.filamentRunout(); }

  void onUserConfirmRequired(const char * const msg) { dgus.confirmationRequest(msg); }

  // For fancy LCDs include an icon ID, message, and translated button title
  void onUserConfirmRequired(const int icon, const char * const cstr, FSTR_P const fBtn) {
    onUserConfirmRequired(cstr);
    UNUSED(icon); UNUSED(fBtn);
  }
  void onUserConfirmRequired(const int icon, FSTR_P const fstr, FSTR_P const fBtn) {
    onUserConfirmRequired(fstr);
    UNUSED(icon); UNUSED(fBtn);
  }

  #if ENABLED(ADVANCED_PAUSE_FEATURE)
    void onPauseMode(
      const PauseMessage message,
      const PauseMode mode/*=PAUSE_MODE_SAME*/,
      const uint8_t extruder/*=active_extruder*/
    ) {
      stdOnPauseMode(message, mode, extruder);
    }
  #endif

  void onStatusChanged(const char * const msg)       { dgus.statusChange(msg); }

  void onHomingStart()    { dgus.homingStart(); }
  void onHomingDone()     { dgus.homingComplete(); }

  void onFactoryReset() {
    dgus.page_index_now = 121;
    dgus.lcd_info.audio_on = DISABLED(SPEAKER);
  }

  void onStoreSettings(char *buff) {
    // Called when saving to EEPROM (i.e. M500). If the ExtUI needs
    // permanent data to be stored, it can write up to eeprom_data_size bytes
    // into buff.

    static_assert(sizeof(dgus.lcd_info) <= ExtUI::eeprom_data_size);
    memcpy(buff, &dgus.lcd_info, sizeof(dgus.lcd_info));
  }

  void onLoadSettings(const char *buff) {
    // Called while loading settings from EEPROM. If the ExtUI
    // needs to retrieve data, it should copy up to eeprom_data_size bytes
    // from buff

    static_assert(sizeof(dgus.lcd_info) <= ExtUI::eeprom_data_size);
    memcpy(&dgus.lcd_info, buff, sizeof(dgus.lcd_info));
    memcpy(&dgus.lcd_info_back, buff, sizeof(dgus.lcd_info_back));
  }

  void onPostprocessSettings() {
    // Called after loading or resetting stored settings
    dgus.paramInit();
    dgus.powerLoss();
  }

  void onSettingsStored(const bool success) {
    // Called after the entire EEPROM has been written,
    // whether successful or not.
  }

  void onSettingsLoaded(const bool success) {
    // Called after the entire EEPROM has been read,
    // whether successful or not.
  }

  #if HAS_LEVELING
    void onLevelingStart() {}
    void onLevelingDone() {}
    #if ENABLED(PREHEAT_BEFORE_LEVELING)
      celsius_t getLevelingBedTemp() { return LEVELING_BED_TEMP; }
    #endif
  #endif

  #if HAS_MESH
    void onMeshUpdate(const int8_t xpos, const int8_t ypos, const_float_t zval) {
      // Called when any mesh points are updated
      //SERIAL_ECHOLNPGM("onMeshUpdate() x:", xpos, " y:", ypos, " z:", zval);
    }

    void onMeshUpdate(const int8_t xpos, const int8_t ypos, const probe_state_t state) {
      // Called to indicate a special condition
      //SERIAL_ECHOLNPGM("onMeshUpdate() x:", xpos, " y:", ypos, " state:", state);
    }
  #endif

  #if ENABLED(PREVENT_COLD_EXTRUSION)
    void onSetMinExtrusionTemp(const celsius_t) {}
  #endif

  #if ENABLED(POWER_LOSS_RECOVERY)
    // Called when power-loss is enabled/disabled
    void onSetPowerLoss(const bool) { dgus.powerLoss(); }
    // Called when power-loss state is detected
    void onPowerLoss() { /* handled internally */ }
    // Called on resume from power-loss
    void onPowerLossResume() { dgus.powerLossRecovery(); }
  #endif

  #if HAS_PID_HEATING
    void onPIDTuning(const pidresult_t rst) {
      // Called for temperature PID tuning result
    }
    void onStartM303(const int count, const heater_id_t hid, const celsius_t temp) {
      // Called by M303 to update the UI
    }
  #endif

  #if ENABLED(MPC_AUTOTUNE)
    void onMPCTuning(const mpcresult_t rst) {
      // Called for temperature MPC tuning result
    }
  #endif

  #if ENABLED(PLATFORM_M997_SUPPORT)
    void onFirmwareFlash() {}
  #endif

  void onSteppersDisabled() {}
  void onSteppersEnabled() {}
  void onAxisDisabled(const axis_t) {}
  void onAxisEnabled(const axis_t) {}
}

#endif // ANYCUBIC_LCD_VYPER