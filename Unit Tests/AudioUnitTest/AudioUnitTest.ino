#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

// I2S configuration constants
#define I2S_NUM         (0) // I2S port number
#define I2S_SAMPLE_RATE (44100)
#define I2S_BITS        (16)
#define I2S_CHANNELS    (2)

// Function to generate a sine wave audio signal
void generateSineWave(int16_t* buffer, int bufferSize, float frequency, float amplitude) {
//    const float frequency = 440.0;  // Frequency of the sine wave (440 Hz for A4)
 //   const float amplitude = 10000;  // Amplitude of the sine wave

    for (int i = 0; i < bufferSize; i += 2) {
        float sample = amplitude * sin(2 * M_PI * frequency * (i / 2) / I2S_SAMPLE_RATE);
        buffer[i] = buffer[i + 1] = (int16_t) sample;
    }
}

void setup() {
    // Initialize I2S configuration
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX), // Master transmitter
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt level 1
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false  // Set to true if using APLL (ESP32-S2 specific)
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = 18,   // BCK pin
        .ws_io_num = 5,    // LRCK pin
        .data_out_num = 19, // DATA pin
        .data_in_num = I2S_PIN_NO_CHANGE // Not used
    };

    // Install and start I2S driver
    i2s_driver_install((i2s_port_t)I2S_NUM, &i2s_config, 0, NULL);
    i2s_set_pin((i2s_port_t)I2S_NUM, &pin_config);

    // Prepare a buffer to store audio data

}
void loop()
{
    const int bufferSize = 1028;
    int16_t buffer[bufferSize];
    
    // Generate a sine waves
    generateSineWave(buffer, bufferSize, 460, 1500);
    
    size_t bytes_written = 0;
    i2s_write((i2s_port_t)I2S_NUM, buffer, bufferSize * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    
    delay(1000);

    generateSineWave(buffer, bufferSize, 800, 2000);

    bytes_written = 0;
    i2s_write((i2s_port_t)I2S_NUM, buffer, bufferSize * sizeof(int16_t), &bytes_written, portMAX_DELAY);

    delay(1000);

    generateSineWave(buffer, bufferSize, 1300, 3500);

    bytes_written = 0;
    i2s_write((i2s_port_t)I2S_NUM, buffer, bufferSize * sizeof(int16_t), &bytes_written, portMAX_DELAY);

    delay(1000);

    generateSineWave(buffer, bufferSize, 800, 2000);

    bytes_written = 0;
    i2s_write((i2s_port_t)I2S_NUM, buffer, bufferSize * sizeof(int16_t), &bytes_written, portMAX_DELAY);

    delay(1000);

    generateSineWave(buffer, bufferSize, 600, 1500);

    bytes_written = 0;
    i2s_write((i2s_port_t)I2S_NUM, buffer, bufferSize * sizeof(int16_t), &bytes_written, portMAX_DELAY);

}