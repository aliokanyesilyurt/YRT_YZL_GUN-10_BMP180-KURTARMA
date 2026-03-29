#include "functions.h" // Header dosyası eklendi.

UcusFazlari ucusDurumu = FAZ_RAMPA; // Faz sıfırlandı.
uint8_t dusmeSayaci = 0; // Değişkenlerin tamamı tanımlandı ve sıfırlandı.
uint32_t ivmeZamanGecmis = 0, hizZamanGecmis = 0;
float irtifaBagil = 0, irtifaMax = 0, irtifaFiltre = 0, irtifaGuncel = 0, irtifaGecmis = 0,
		irtifaBaslangic = 0, basincFiltre = 0, basincGuncel = 0, basincBaslangic = 0,
		sicaklikFiltre = 0, sicaklikGuncel = 0, hizGecmis = 0, ivmeGecmis = 0,
		hizGuncel = 0, ivmeGuncel = 0, ivmeFiltre = 0, hizFiltre = 0, ivmeDegisimi = 0;
char buffer[120];

void firlatma(void) { // Fırlatmanın tespit edilmesini sağlayan fonksiyon.
	if (basincBaslangic == 0) {
		        basincBaslangic = basincFiltre;
		        irtifaBaslangic = irtifaFiltre;
	} // Basıncın yerde ölçülmesini sağladık ve irtifayı bağılladık.
	if (irtifaBagil > 20 && ivmeFiltre > 5.0f) {
		ucusDurumu = FAZ_FIRLATMA; // İtki tespit edildiğinde itkili fırlatmaya geçtik.
	}
}

void tirmanma(void){
	    ivmeDegisimi = ivmeFiltre - ivmeGecmis;
	    // İvme değişimi ve hız büyüklüğü yardımıyla burnout tespit edildi.
	    if (ivmeDegisimi < -10.0f && hizFiltre > 0.0f) {
	        ucusDurumu = FAZ_TIRMANIS;
	    }
	    if (ivmeFiltre < 3.0f && hizFiltre > 0.0f) {
	        ucusDurumu = FAZ_TIRMANIS;
	    }
	    ivmeGecmis = ivmeFiltre;
}

void arama(void) {
	if(irtifaBagil < M_KILIT){
		dusmeSayaci = 0;
	}else{
		ucusDurumu = FAZ_ARAYIS;
	}
} // Tepe noktası aramaya başlamak için kilidin açılmasını bekledik.

void drogueAcma(void) {
	if (irtifaBagil > irtifaMax) { // Sayacı saydırmaya başladık.
		irtifaMax = irtifaBagil;
		dusmeSayaci = 0;
	} else {
		dusmeSayaci++;
	}
	if (dusmeSayaci > 5) {
		HAL_GPIO_WritePin(TEPE_PA9_GPIO_Port, TEPE_PA9_Pin, 1); // Drogue paraşütü servosu çalıştı paraşüt atıldı.
		HAL_GPIO_WritePin(LED_PA5_GPIO_Port, LED_PA5_Pin, 1); // Kurtarma başlangıç ledi.
		ucusDurumu = FAZ_DUSUS;
	}
}

void anaParasutAcma(void) {
	if (irtifaBagil < M_ANA) { // Ana paraşütün açılma şartını yüksekliğe bağladık.
		HAL_GPIO_WritePin(ANA_PA8_GPIO_Port, ANA_PA8_Pin, 1); // Ana paraşüt servosu açıldı paraşüt atıldı.
		ucusDurumu = FAZ_INIS;
	}
}

void ledYakma(void) {
	if (irtifaBagil < 10) {
		HAL_GPIO_TogglePin(LED_PA5_GPIO_Port, LED_PA5_Pin); // İnince led yakma.
		HAL_Delay(100);
	}
}

float hizHesaplama(void){
	uint32_t zamanGuncel = HAL_GetTick(); // Zaman verisini çekiyoruz.

	float dt = (zamanGuncel - hizZamanGecmis) / 1000.0f; // Bu veriyle türev alıp
	if (dt <=0) return hizGuncel; // basit bölme işlemiyle hız hesaplıyoruz.

	hizGuncel = (irtifaFiltre - irtifaGecmis) / dt;
	    irtifaGecmis = irtifaFiltre;
	    hizZamanGecmis  = zamanGuncel;
	return hizGuncel;
}

float ivmeHesaplama(void){
	uint32_t zamanGuncel = HAL_GetTick();

	float dt = (zamanGuncel - ivmeZamanGecmis) / 1000.0f; // Hız gibi de burada
	if (dt <=0) return ivmeGuncel; // ivme hesaplıyoruz.

	ivmeGuncel = (hizGuncel - hizGecmis) / dt;
	    hizGecmis = hizGuncel;
	    ivmeZamanGecmis  = zamanGuncel;
	return ivmeGuncel;
}

float irtifaHesaplama(void) {
    irtifaGuncel = 44330.0f * (1.0f - powf(basincFiltre / basincBaslangic, 0.1903f));
    return irtifaGuncel; // Basınç ve sıcaklıktan irtifa hesabı formülü.
}

void inisKontrol(void) {
    if (irtifaBagil < M_YEDEK && hizFiltre < -15.0f && ivmeFiltre < -5.0f) {
        HAL_GPIO_WritePin(YEDEK_PA7_GPIO_Port, YEDEK_PA7_Pin, 1);
    }
    if (irtifaBagil < 20) {
        ucusDurumu = FAZ_BITIS;
    } // İnişin doğruluğunu kontrol edip yedek paraşüt açan fonksiyon.
}

float ortFiltreleme(float ortGuncel, float ortFiltre) { // Tüm verilerimizin
	if (ortFiltre == 0.0) { // ortalamasını tek fonksiyonda hesaplamaya çalıştım.
		ortFiltre = ortGuncel;
	} else {
		ortFiltre = (ortGuncel * 0.1) + (ortFiltre * 0.9);
	}
	return ortFiltre;
}

void degerAtama(void){ // Tüm değerleri tek fonksiyonda yenileyen fonksiyonumuz.
	basincGuncel = BMP180_GetPressure();
	sicaklikGuncel = BMP180_GetTemperature();
	basincFiltre = ortFiltreleme(basincGuncel, basincFiltre);
	sicaklikFiltre = ortFiltreleme(sicaklikGuncel, sicaklikFiltre);
	// Önce basınç sıcaklık sonra filtrel irtifa.
	if (basincBaslangic > 0) irtifaGuncel = irtifaHesaplama();
	irtifaFiltre = ortFiltreleme(irtifaGuncel, irtifaFiltre);
	// Sonra hız çünkü yukarıdakiler olmadan hız hesaplanmıyor.
	hizGuncel = hizHesaplama();
	ivmeGuncel = ivmeHesaplama();
	// En son ise bunların filtreli hali ve bağıl irtifa formülü.
	hizFiltre = ortFiltreleme(hizGuncel, hizFiltre);
	ivmeFiltre = ortFiltreleme(ivmeGuncel, ivmeFiltre);
	irtifaBagil = irtifaFiltre - irtifaBaslangic;
	if (irtifaBagil < 0) irtifaBagil = 0;
}

void uartOkuma(void){ // UART'a veri aktarmamızı sağlayan fonksiyon bu sayede test yaparken ekranda bu büyüklükleri göreceğiz.
	sprintf(buffer, "Filtreli-Ortalama Veriler --- Faz:%d | İrtifa:%.2f | Hız:%.2f | İvme:%.2f | Basınç:%.2f\r\n",
	        ucusDurumu, irtifaBagil, hizFiltre, ivmeFiltre, basincFiltre);
	HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
}
