# sh ./build-repeaters-iotthinks.sh
export FIRMWARE_VERSION="PowerSaving13"

# Commonly-used boards
## ESP32
sh build.sh build-firmware \
Heltec_v3_repeater \
Heltec_WSL3_repeater \
heltec_v4_repeater \
Station_G2_repeater \
T_Beam_S3_Supreme_SX1262_repeater \
Tbeam_SX1262_repeater

## NRF52
sh build.sh build-firmware \
RAK_4631_repeater \
Heltec_t114_repeater \
Xiao_nrf52_repeater \
Heltec_mesh_solar_repeater \
ProMicro_repeater \
SenseCap_Solar_repeater

## SX1276
sh build.sh build-firmware \
Heltec_v2_repeater \
LilyGo_TLora_V2_1_1_6_repeater \
Tbeam_SX1276_repeater

# Newly-supported boards
sh build.sh build-firmware \
Xiao_S3_WIO_repeater \
Xiao_C3_repeater \
Xiao_C6_repeater_ \
RAK_3401_repeater \
Heltec_E290_repeater_