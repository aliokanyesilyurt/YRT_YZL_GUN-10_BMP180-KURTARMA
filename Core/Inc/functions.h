#ifndef FUNCTIONS_H
#define FUNCTIONS_H
// Define kontrolleri eklendi.
#define M_KILIT 5000
#define M_TEPE 8000
#define M_ANA 2000
#define M_YEDEK 500
#define SAYAC 5
#define FILTRE 10
#define DENIZ_BAS 101325.0f

#include "main.h" // Fonksiyonların çalışması için gereken kütüphane.
#include "math.h" // Tüm gerekli diğer kütüphaneler buraya eklendi.
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "bmp180_for_stm32_hal.h" // Basınç fonksiyonlarını kullanmak için.
// Enumlarımız burada.
typedef enum {
    FAZ_RAMPA, // Rampa konumunda.
	FAZ_FIRLATMA, // İtkili atış kısmı.
    FAZ_TIRMANIS, // Apogee atamaz kilidi var.
	FAZ_ARAYIS, // Kilit kalktı ve apogee atabilir.
    FAZ_DUSUS, // Sürüklenme sonrası durum. Bu fazın sonunda ana paraşütü açacak.
    FAZ_INIS, // Ana paraşüt sonrası alçalma durumu.
	FAZ_BITIS // Yere tam olarak indiğini yahut hızının vs sıfırlandığını belirten durum.
} UcusFazlari;

// Tekrar kullanmasın mainden baksın diye extern ile attım.
extern UcusFazlari ucusDurumu;
extern UART_HandleTypeDef huart2;
// Değişkenlerimizi tanımladım ve sıfırladım.
extern uint8_t dusmeSayaci;
extern uint32_t ivmeZamanGecmis, hizZamanGecmis;
extern float irtifaBagil, irtifaMax, irtifaFiltre, irtifaGuncel, irtifaGecmis,
		irtifaBaslangic, basincFiltre, basincGuncel, basincBaslangic,
		sicaklikFiltre, sicaklikGuncel, hizGecmis, hizGuncel,
		ivmeGuncel, ivmeFiltre, hizFiltre, ivmeGecmis, ivmeDegisimi;
extern char buffer[120];

// Kullanılan fonksiyonlar burada.
// Virgüllerle uğraşacağımız için genellikle sayı döndürenlere float tercih ettim.
// İşlem yapanlar zaten döndürme yapmıyor onlar void.
void firlatma(void); // Kalkışın tespitini sağlayan fonksiyon.
void tirmanma(void); // Burnout tespiti yapan fonksiyon.
void arama(void); // Kalkış yapıldıysa apogee aramayı burada başlat.
void drogueAcma(void); // Apogee bulunduysa paraşütü bununla aç.
void anaParasutAcma(void); // Hız istenen duruma geldiyse ana paraşütü bununla aç.
void ledYakma(void); // İniş tamamen yapıldıysa bununla led yak.
void inisKontrol(void); // İnişin başarılı olup olmadığını kontrol eden fonksiyon.
float hizHesaplama(void); // İnişin tamamlanıp tamamlanmadığını ve hızın istenen düzeyde yavaşlayıp yavaşlamadığını bununla kontrol et.
float ivmeHesaplama(void); // İvme hesaplama yapan fonksiyon.
float ortFiltreleme(float ortGuncel, float ortFiltre); // Hareketli ortalama filtresi.
void degerAtama(void); // Tüm değerlerimizi güncellememizi sağlayan fonksiyon.
void uartOkuma(void); // UART ile ekranımıza veri gönderen fonksiyon.
#endif
