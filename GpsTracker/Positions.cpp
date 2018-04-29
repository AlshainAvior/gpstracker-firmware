#include "Config.h"
#include "Debug.h"
#include "Positions.h"
#include "Gps.h"

#if BACKUP_ENABLE_SDCARD || BACKUP_ENABLE_NETWORK
#define BACKUPS_ENABLED BACKUP_ENABLE_SDCARD + BACKUP_ENABLE_NETWORK
#endif

#if BACKUP_ENABLE_SDCARD
#include "SdPositionsBackup.h"
#endif

#if BACKUP_ENABLE_NETWORK
#include "NetworkPositionsBackup.h"
#endif

#define LOGGER_NAME "Positions"

#define ENTRY_RESERVED_SIZE	128
#define ENTRIES_ADDR		CONFIG_RESERVED_SIZE

namespace positions {
#ifdef BACKUPS_ENABLED
	backup::PositionsBackup **_backups;
#endif

	namespace details {
		uint16_t maxEntryIndex = 0;

		uint16_t getEntryAddress(uint16_t index) {
			if (index > maxEntryIndex) return -1;
			return ENTRIES_ADDR + (ENTRY_RESERVED_SIZE * index);
		}
	}

	void setup() {
		details::maxEntryIndex = (E24_MAX_ADDRESS(hardware::i2c::eeprom.getSize()) - ENTRIES_ADDR) / ENTRY_RESERVED_SIZE;
#ifdef BACKUPS_ENABLED
		uint8_t backupIdx = 0;
		_backups = new backup::PositionsBackup*[BACKUPS_ENABLED];

#if BACKUP_ENABLE_SDCARD
		_backups[backupIdx] = new backup::sd::SdPositionsBackup();
		_backups[backupIdx]->setup();
		backupIdx++;
#endif
#if BACKUP_ENABLE_NETWORK
		_backups[backupIdx] = new backup::net::NetworkPositionsBackup();
		_backups[backupIdx]->setup();
		backupIdx++;
#endif
#endif
	}

	bool acquire(PositionEntryMetadata &metadata) {
		NOTICE("acquire");

		timestamp_t before;

		gps::powerOn();
		before = rtc::getTime();
		SIM808_GPS_STATUS gpsStatus = gps::acquireCurrentPosition(GPS_DEFAULT_TOTAL_TIMEOUT_MS);
		SIM808ChargingStatus battery = hardware::sim808::device.getChargingState();
		gps::powerOff();

		NOTICE_FORMAT("acquire", "Status : %d", gpsStatus);

		if (gpsStatus < SIM808_GPS_STATUS::FIX) return false;

		uint16_t timeToFix = rtc::getTime() - before;

		tmElements_t time;
		gps::getTime(time);
		rtc::setTime(time);

		metadata = {
			battery.level,
			battery.voltage,
			rtc::getTemperature(),
			timeToFix,
			gpsStatus
		};

		return true;
	}

	void appendLast(const PositionEntryMetadata &metadata) {
		VERBOSE("appendLast");

		uint16_t entryIndex;
		uint16_t entryAddress;
		PositionEntry entry = { metadata };
		strlcpy(entry.position, gps::lastPosition, POSITION_SIZE);

		config_t* config = &config::main::value;
		entryIndex = config->lastEntry + 1;

		entryAddress = details::getEntryAddress(entryIndex);

		hardware::i2c::powerOn();
		hardware::i2c::eeprom.writeBlock(entryAddress, entry);

		NOTICE_FORMAT("appendLast", "Saved @ %X : [%d%% @ %dmV] [%f�C] [TTF : %d, Status : %d, Position : %s]", entryAddress, entry.metadata.batteryLevel, entry.metadata.batteryVoltage, entry.metadata.temperature, entry.metadata.timeToFix, entry.metadata.status, entry.position);

		config->lastEntry++;
		if (config->lastEntry > details::maxEntryIndex) config->lastEntry = 0;
		if (config->lastEntry == config->firstEntry) config->firstEntry++;
		if (config->firstEntry > details::maxEntryIndex) config->firstEntry = 0;

		config::main::save();
		hardware::i2c::powerOff();
	}

	bool get(uint16_t index, PositionEntry &entry) {
		VERBOSE("get");

		uint16_t entryAddress = details::getEntryAddress(index);
		if (entryAddress == -1) return false;

		VERBOSE_FORMAT("get", "Reading entry n�%d @ %X", index, entryAddress);

		hardware::i2c::powerOn();
		hardware::i2c::eeprom.readBlock(entryAddress, entry);
		hardware::i2c::powerOff();

		NOTICE_FORMAT("get", "Read from EEPROM @ %X : [%d%% @ %dmV] [%f�C] [TTF : %d, Status : %d, Position : %s]", entryAddress, entry.metadata.batteryLevel, entry.metadata.batteryVoltage, entry.metadata.temperature, entry.metadata.timeToFix, entry.metadata.status, entry.position);
		return true;
	}

	bool moveNext(uint16_t &index) {
		if (index == config::main::value.lastEntry) return false;
		
		if (index == details::maxEntryIndex) index = 0; //could use a modulo but easier to understand that way
		else index++;

		return true;
	}

	uint16_t count(uint16_t fromIndex) {
		config_t *config = &config::main::value;
		uint16_t lastEntry = config->lastEntry;

		if (lastEntry < fromIndex) { lastEntry += details::maxEntryIndex; }

		return lastEntry - fromIndex;
	}

	void prepareBackup() {
#ifdef BACKUPS_ENABLED
		for (int i = 0; i < BACKUPS_ENABLED; i++) {
			_backups[i]->prepare();
		}
#endif
	}

	void doBackup() {
#ifdef BACKUPS_ENABLED
		for (int i = 0; i < BACKUPS_ENABLED; i++) {
			_backups[i]->backup();
		}
#endif
	}
}