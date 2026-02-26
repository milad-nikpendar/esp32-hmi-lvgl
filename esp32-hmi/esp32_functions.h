/*
   esp32_functions header V1.0.0
   Created by Milad Nikpendar
*/
#ifndef ESP32_FUNCTIONS_H
#define ESP32_FUNCTIONS_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <FS.h>
#include <SPIFFS.h>

String humanReadableSize(const size_t bytes, int num = 0)
{
  if (bytes < 1024)
    return String((float)bytes, num) + " B";
  else if (bytes < (1024 * 1024))
    return String((float)bytes / 1024.0, num) + " KB";
  else if (bytes < (1024 * 1024 * 1024))
    return String((float)bytes / 1024.0 / 1024.0, num) + " MB";
  else
    return String((float)bytes / 1024.0 / 1024.0 / 1024.0, num) + " GB";
}

void displayFreeHeap()
{
  // Heap info
  size_t heapSize = ESP.getHeapSize();
  size_t freeHeap = ESP.getFreeHeap();
  size_t maxAllocHeap = ESP.getMaxAllocHeap();

  Serial.printf("[HEAP]  Total: %s\tFree: %s\tMaxBlock: %s\tUsed: %s (%.1f%%)\n",
                humanReadableSize(heapSize, 2).c_str(),
                humanReadableSize(freeHeap, 2).c_str(),
                humanReadableSize(maxAllocHeap, 2).c_str(),
                humanReadableSize(heapSize - freeHeap, 2).c_str(),
                (float)(heapSize - freeHeap) * 100.0 / heapSize);
}

void displayFreePsram()
{
  // PSRAM info
  size_t psramSize = ESP.getPsramSize();
  size_t freePsram = ESP.getFreePsram();
  size_t maxAllocPsram = ESP.getMaxAllocPsram();

  Serial.printf("[PSRAM] Total: %s\tFree: %s\tMaxBlock: %s\tUsed: %s (%.1f%%)\n",
                humanReadableSize(psramSize, 2).c_str(),
                humanReadableSize(freePsram, 2).c_str(),
                humanReadableSize(maxAllocPsram, 2).c_str(),
                humanReadableSize(psramSize - freePsram, 2).c_str(),
                (float)(psramSize - freePsram) * 100.0 / psramSize);
}

void displayFreeSpiffs() {
  if (!SPIFFS.begin(true)) {
    Serial.println("[SPIFFS] Mount failed!");
    return;
  }

  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes  = SPIFFS.usedBytes();
  size_t freeBytes  = totalBytes - usedBytes;

  Serial.printf("[SPIFFS] Total: %s\tUsed: %s\tFree: %s\t(%.1f%% used)\n",
                humanReadableSize(totalBytes, 2).c_str(),
                humanReadableSize(usedBytes, 2).c_str(),
                humanReadableSize(freeBytes, 2).c_str(),
                (float)usedBytes * 100.0 / totalBytes);
}

const char *readMacAddress()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  delay(50);

  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  static char macStr[18];

  if (ret == ESP_OK)
  {
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             baseMac[0], baseMac[1], baseMac[2],
             baseMac[3], baseMac[4], baseMac[5]);
    return macStr;
  }
  return "Failed to read MAC address";
}

String chipID()
{
  uint32_t chipId = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  uint8_t baseMac[6];
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X%02X%02X", baseMac[3], baseMac[4], baseMac[5]);

  return String(chipId) + String(baseMacChr);
}
void chipInfo()
{
  Serial.printf("ESP32 Chip model = %s Rev %d\n", ESP.getChipModel(), ESP.getChipRevision());
  Serial.printf("This chip has %d cores\n", ESP.getChipCores());
  Serial.print("Chip ID: \t");
  Serial.println(chipID());
  Serial.print("SDK: \t");
  Serial.println(ESP.getSdkVersion());
  Serial.print("CPU FREQ: \t");
  Serial.print(getCpuFrequencyMhz());
  Serial.println("MHz");
  Serial.print("APB FREQ: \t");
  Serial.print(getApbFrequency() / 1000000.0, 1);
  Serial.println("MHz");
  Serial.print("FLASH SIZE: \t");
  Serial.println(humanReadableSize(ESP.getFlashChipSize(), 3));
  Serial.print("OTA Size: \t");
  Serial.println(humanReadableSize(ESP.getFreeSketchSpace(), 3));
  Serial.print("Sketch Size: \t");
  Serial.println(humanReadableSize(ESP.getSketchSize(), 3));
  Serial.print("RAM SIZE: \t");
  Serial.println(humanReadableSize(ESP.getHeapSize(), 0));
  Serial.print("FREE RAM: \t");
  Serial.println(humanReadableSize(ESP.getFreeHeap(), 0));
  Serial.print("MAX RAM ALLOC: \t");
  Serial.println(humanReadableSize(ESP.getMaxAllocHeap(), 0));
  Serial.print("FREE PSRAM: \t");
  Serial.println(humanReadableSize(ESP.getFreePsram(), 0));
}

// Function to format size in bytes to human-readable format
const char *formatSize(uint64_t bytes)
{
  static char output[20];
  if (bytes < 1024)
    snprintf(output, sizeof(output), "%llu B", bytes);
  else if (bytes < 1024 * 1024)
    snprintf(output, sizeof(output), "%.1f KB", bytes / 1024.0);
  else if (bytes < 1024 * 1024 * 1024)
    snprintf(output, sizeof(output), "%.1f MB", bytes / (1024.0 * 1024));
  else
    snprintf(output, sizeof(output), "%.1f GB", bytes / (1024.0 * 1024 * 1024));

  return output;
}
#endif //ESP32_FUNCTIONS_H
