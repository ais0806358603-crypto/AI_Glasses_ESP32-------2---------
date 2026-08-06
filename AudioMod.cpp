#include "AudioMod.h"

AudioMod Audio;

AudioMod::AudioMod() {
    _isInitialized = false;
}

bool AudioMod::begin() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 24000, // Typical for Google TTS
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // MAX98357A is usually mono
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .mck_io_num = I2S_PIN_NO_CHANGE,
        .bck_io_num = I2S_SPK_BCLK,
        .ws_io_num = I2S_SPK_LRC,
        .data_out_num = I2S_SPK_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    esp_err_t err = i2s_driver_install(I2S_PORT_SPEAKER, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed installing I2S driver for speaker: %d\n", err);
        return false;
    }

    err = i2s_set_pin(I2S_PORT_SPEAKER, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed setting I2S pins for speaker: %d\n", err);
        return false;
    }

    _isInitialized = true;
    return true;
}

void AudioMod::playSystemSound(const String& soundName) {
    if (!_isInitialized) return;
    Serial.println("Playing system sound: " + soundName);
    // In a real implementation, you would read a WAV/MP3 file from SD card
    // and write the PCM data to I2S via i2s_write().
    // For now, we simulate this.
}

void AudioMod::feedTTSData(const uint8_t* data, size_t length) {
    if (!_isInitialized) return;
    size_t bytesWritten;
    i2s_write(I2S_PORT_SPEAKER, data, length, &bytesWritten, portMAX_DELAY);
}
