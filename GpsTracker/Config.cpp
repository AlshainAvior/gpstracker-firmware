#include "Config.h"
#include "Debug.h"
#include "Hardware.h"

#define LOGGER_NAME "Config"

namespace config {
	namespace main {

		config_t value;

		namespace details {

			void read() {
				VERBOSE("read");
				hardware::i2c::powerOn();
				hardware::i2c::eeprom.readBlock(CONFIG_ADDR, value);
				if (CONFIG_SEED != value.seed) reset(); //todo : reset network if seed for network is not right
				hardware::i2c::powerOff();

				NOTICE_FORMAT("read", "%d, %s, %d, %d", value.seed, value.version, value.firstEntry, value.lastEntry);
#if BACKUP_ENABLE_NETWORK
				VERBOSE_FORMAT("read", "%d, %d, %s, %s", value.network.saveThreshold, value.network.lastSavedEntry, value.network.apn, value.network.url);
#endif
			}

			void write() {
				NOTICE_FORMAT("write", "%d, %s, %d, %d", value.seed, value.version, value.firstEntry, value.lastEntry);
#if BACKUP_ENABLE_NETWORK
				VERBOSE_FORMAT("write", "%d, %d, %s, %s", value.network.saveThreshold, value.network.lastSavedEntry, value.network.apn, value.network.url);
#endif
				hardware::i2c::powerOn();
				int written = hardware::i2c::eeprom.writeBlock(CONFIG_ADDR, value);
				hardware::i2c::powerOff();
			}
		}

		void setup() {
			details::read();
		}

		void save() {
			details::write();
		}

		void reset() {
			VERBOSE("reset");
			config_t config = {
				CONFIG_SEED,
				VERSION,
				0xFFFF,
				0xFFFF,
#if BACKUP_ENABLE_NETWORK
				{
					POSITIONS_CONFIG_NET_DEFAULT_SAVE_THRESHOLD,
					0xFFFF,
					POSITIONS_CONFIG_NET_DEFAULT_APN,
					POSITIONS_CONFIG_NET_DEFAULT_URL,
				},
#endif
			};

			value = config;
			save();
		}

	}
}