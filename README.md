# IoT Väderövervakning med ESP32 och DHT11

## Innehållsförteckning
- [Introduktion](#introduktion)
- [Komponenter](#komponenter)
- [Intruktioner](#instruktioner)
- [Flödesschema](#flödesschema)
- [Säkerhet och Skalbarhet](#säkerhet-och-skalbarhet)
- [Slutsats](#slutsats)

---
## Introduktion
Detta projekt samlar in data från en DHT11-sensor via en ESP32-enhet. Sensordata (temperatur och luftfuktighet) skickas till AWS IoT Core för vidare hantering. Datan lagras i DynamoDB och visualiseras i realtid i Grafana. Vid statusändringar (ansluten/frånkopplad) skickas notifikationer via Telegram.
Projektet är designat för att möta behovet av realtidsövervakning av inomhusklimat, exempelvis i växthus, bostäder eller kontor. Fokus har lagts på säkerhet, tillgänglighet och enkel hantering av data. Systemet är skalbart och kan anpassas för fler sensorer eller integration med andra IoT-enheter.

--- 

## Komponenter

- **DHT11 Sensor**: Sensor för att mäta temperatur och luftfuktighet.
- **ESP32**: Mikrokontroller för att hantera sensordata och kommunicera med AWS IoT Core.
- **AWS IoT Core**: Hanterar anslutning och autentisering av enheter via MQTT.
- **DynamoDB**: Lagrar sensordata.
- **Grafana**: Visualiserar de insamlade data i realtid.
- **Telegram**: Skickar statusnotifikationer vid anslutning och frånkoppling av enheter.
- **AWS Lambda**: Processar statusändringar och skickar meddelanden till Telegram.

---

## 1. **Sensor (DHT11 & ESP32)**

- **Enhet:** DHT11 ansluten till ESP32.
- **Funktion:** Samlar in temperatur och luftfuktighetsdata och skickar den var 10:e sekund via MQTT.
<p align="center">
  <img src="Images/ESP32.jpg" width="300">
</p>

---

## 2. **Dataöverföring**

- **Protokoll:** MQTT (säker kommunikation via TLS).
- **Topic (Ämne):** `/telemetry`
- **Payload Format:**
    ```json
    {
      "temperature": 23.8,
      "humidity": 20.0
    }
    ```
- **Säkerhet:** Användning av X.509-certifikat för autentisering och TLS för säker dataöverföring.

---

## 3. **Molnhantering**

### **AWS IoT Core**
- **Autentisering:** X.509-certifikat för enhetsautentisering.
- **Regler:** 
  - **Datahantering:** Inkommande data vidarebefordras till DynamoDB för lagring.
  - **Status:** AWS IoT Core skickar statusuppdateringar till AWS Lambda (exempelvis enhetsstatus: ansluten/frånkopplad).

### **DynamoDB**
- **Syfte:** Lagring av sensordata för framtida analys.
- **Struktur:** 
  - Partition key: `deviceId`
  - Attribut: `timestamp`, `temperature`, `humidity`

---

## 4. **Visualisering**

- **Verktyg:** Grafana.
- **Datakälla:** DynamoDB.
- **Dashboard-funktioner:**
  - Real-time grafer för temperatur och luftfuktighet.
  - Uppdateras varje 10:e sekund för att visa aktuella mätvärden.
  ![Grafana](Images/Grafana.png)

---

## 5. **Notifikationer via Telegram**

- **Tjänst:** Telegram.
- **Trigger:** AWS Lambda triggas vid förändringar i enhetens anslutning (ansluten eller frånkopplad).
- **Exempelmeddelande:** 
  - `"Enhet kopplades från!"`
- **Meddelandeformat:** Notifikationer skickas i realtid för att informera om enhetens statusändring.

<p align="center">
  <img src="Images/Telegram.png" width="200">
</p>

---

## **Instruktioner**

1. **Förbered DHT11-sensorn:**
   - Anslut DHT11-sensorn till **ESP32** enligt dokumentationen för din specifika sensor.
   - Se till att den är korrekt ansluten till en GPIO-port på ESP32, till exempel GPIO21.

2. **Koppla upp ESP32 till Wi-Fi:**
   - I koden, ersätt `WIFI_SSID` och `WIFI_PASSWORD` med dina egna Wi-Fi-uppgifter.
   - Se till att ESP32 har en stabil internetanslutning för att kommunicera med **AWS IoT Core**.

3. **Konfigurera AWS IoT Core:**
   - Skapa en **IoT-thing** i **AWS IoT Core**.
   - Hämta de nödvändiga certifikaten för autentisering (X.509-certifikat).
   - Lägg till certifikaten i din ESP32-kod för att säkerställa autentisering med AWS.

4. **Skicka data till AWS IoT Core:**
   - Använd **MQTT-protokollet** för att skicka sensorvärden (temperatur och luftfuktighet) till **AWS IoT Core**.
   - Se till att ESP32 publicerar data till rätt MQTT-ämne (`/telemetry`).

5. **Lagra data i DynamoDB:**
   - Konfigurera **DynamoDB** för att ta emot och lagra sensordata från **AWS IoT Core**.
   - Skapa tabeller för att lagra data baserat på unika enhets-ID:n och tidsstämplar.

6. **Visualisera data i Grafana:**
   - Konfigurera **Grafana** att hämta data från **DynamoDB**.
   - Skapa ett dashboard för att visa temperatur och luftfuktighet i realtid.

7. **Ställ in Telegram-notifikationer:**
   - Skapa en **AWS Lambda-funktion** för att skicka statusuppdateringar (ansluten/frånkopplad) när enheten ansluter eller kopplas från **AWS IoT Core**.
   - Anslut Lambda-funktionen till **Telegram Bot API** för att skicka meddelanden till användaren om enhetens status.

8. **Testa och övervaka projektet:**
   - Starta projektet och övervaka i **Grafana** för att se temperatur- och luftfuktighetsvärden.
   - Kontrollera att Telegram skickar notifieringar när enheten ansluter eller kopplas från.


---

## **Flödesschema**
![Flöde](Images/FlödeDiagram.png)

---

## **Säkerhet och Skalbarhet**


- **Säkerhet**:  
  Lösningen implementerar flera säkerhetsåtgärder för att skydda data och kommunikation.  
  - **TLS-kryptering** används för säker dataöverföring mellan enheter och molntjänster.  
  - **X.509-certifikat** används för att autentisera enheter, vilket förhindrar obehörig åtkomst.  
  - **IAM Policies och AWS IoT Rules** är konfigurerade för att styra vilka enheter och användare som har åtkomst till specifika resurser, samt för att begränsa datatrafik till fördefinierade ämnen (topics).  
  - Datan lagras säkert i AWS DynamoDB med kryptering aktiverad och är endast åtkomlig via reglerade API-anrop. 
 
- **Skalbarhet**:  
  Systemet är byggt för att kunna växa med antalet enheter och data utan att förlora funktionalitet eller säkerhet.  
  - **AWS IoT Core** kan hantera tusentals anslutna enheter samtidigt.  
  - **DynamoDB** skalar horisontellt, vilket innebär att ökade datamängder inte påverkar prestandan.  
  - Visualisering i **Grafana** kan enkelt utökas för att inkludera fler datakällor och dashboards.  
  - Policies och regler gör det möjligt att hantera och övervaka många sensorer och enheter effektivt.  

- **Människocentrerad design**: Projektet adresserar behovet av tillgänglig och pålitlig klimatövervakning. Det kan användas i växthus för att skydda växter eller i bostäder för att säkerställa ett behagligt och hälsosamt inomhusklimat.



### Resonemang:
**Hög säkerhet uppnås** genom att kombinera kryptering, stark autentisering och noggrann åtkomstkontroll med hjälp av policies och regler. **Skalbarheten** säkerställs genom molntjänster som dynamiskt hanterar ökade datamängder och fler enheter utan att kompromissa med prestanda eller säkerhet.

---

## **Slutsats**

Projektet demonstrerar en effektiv och säker lösning för att samla in, lagra och visualisera sensorinformation i realtid. Genom att använda **AWS IoT Core** för enhetskommunikation, **DynamoDB** för säker och skalbar lagring av sensorvärden och **Grafana** för användarvänlig visualisering, säkerställs både pålitlig datahantering och intuitiv övervakning.

För lagring av sensorvärden som temperatur och luftfuktighet valdes **DynamoDB** på grund av dess förmåga att hantera små, frekventa datapunkter med snabb åtkomst, vilket är avgörande för realtidsapplikationer. **Amazon S3**, som är mer lämpad för lagring av stora filer och ostrukturerad data, skulle inte ha uppfyllt projektets krav på snabb och frekvent åtkomst till små datamängder.

Lösningen är skalbar, vilket gör det möjligt att enkelt utöka systemet med fler sensorer eller enheter vid behov. Systemet kan även enkelt anpassas för framtida utvidgningar eller förändrade krav, vilket gör det till en flexibel och långsiktig lösning för klimatövervakning och andra IoT-tillämpningar.


