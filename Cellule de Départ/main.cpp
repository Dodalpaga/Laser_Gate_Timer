#include "mbed.h"
#include "ble/BLE.h"
#include "ble/services/UARTService.h"
#include "Serial.h"
#include "stdlib.h"

#define UART_BUFFER (UARTService::BLE_UART_SERVICE_MAX_DATA_LEN)


const static char DEVICE_NAME[] = "ObCP";
UARTService *uartServicePtr;

///////////Périphériques/////////////////
InterruptIn myButton(USER_BUTTON);
InterruptIn photodiode(A2);
DigitalOut myled(LED1);
DigitalOut laser(A3);
DigitalIn photodiode1(A2);
DigitalOut compteur(A4);
float photodiode2 = 0;

///////////Init devices//////////////////
Serial device(D8, D2);
Serial pc(USBTX, USBRX);

//////////Initialisations////////////////
Timer timer_player_1;
Timer timer_player_2;
int begin1;
int begin2;
int end1;
int end2;
float chrono1;
float chrono2;
char temp=0;
char temp2=0;
char temp3=0;
char gamemode = 'M';

void Timer_Reset()
{
    timer_player_1.stop();
    timer_player_2.stop();
    timer_player_1.reset();
    timer_player_2.reset();
    begin1=0;
    begin2=0;
    end1=0;
    end2=0;
    chrono1=0;
    chrono2=0;
}

void photodiode_triggered()
{
    if (gamemode == 'S') {
        compteur = 1;
        wait(0.001);
        compteur = 0;
        Timer_Reset();
        timer_player_1.start();
        begin1 = timer_player_1.read_ms();
        myled=1;
        wait(0.5);
        myled=0;
        
    }
    if (gamemode == 'D') {
        wait(0.001);
        compteur = 0;
        end2 = timer_player_2.read_ms();
        timer_player_2.stop();
        chrono2 = end2-begin2;
    }
    if (gamemode == 'L') {
        wait(0.001);
        compteur = 0;
        timer_player_1.start();
        chrono1 = timer_player_1.read_ms();
        myled=1;
        wait(0.5);
        myled=0;
    }
}
/////////////////////////////////////


/////////COMMUNICATION///////////////
void device_receive()
{
    temp=device.getc();
    if (temp!=0xAA) {
        if(temp == 0xBB and temp2!=0xBB) {
            temp2=0xBB;
        }
        if(temp == 0xBC and temp2==0xBB) {
            temp3 = 0xBC;
        }
        if(temp == 0xBD and temp3==0xBC) {
            end1 = timer_player_1.read_ms();
            timer_player_1.stop();
            chrono1 = end1-begin1;
            photodiode2 = 0;
            pc.printf("C'est bon");
        }
        if(temp == 0xCC) {
            photodiode2 = 1;
        }
        pc.putc(temp);
    }


}
/////////////////////////////////////


// Tableau et index de communication avec UART
static char uartBuff[UART_BUFFER];
// Routine de traitement des erreurs
void onBleError(ble_error_t error);

/****************************************************/
/* Ticker actif lorsque la connexion BLE est présente */
/****************************************************/
void ConnectedBLE(void)
{
    // Signal de vie: allumer et éteindre la LED

}

void update(void)
{

    if (gamemode != 'A') {
        char train1[5];
        char train2[5];
        float chrono_value_J1 = float(chrono1-30)* 0.001F;
        float chrono_value_J2 = float(chrono2)* 0.001F;

        //Transformation des valeurs numeriques en chaine de caracteres
        sprintf(train1,"%5.2f",chrono_value_J1);
        sprintf(train2,"%5.2f",chrono_value_J2);
        //Integre les trois chaines de caractere contenant les ts dans la chaine uartBuff
        sprintf(uartBuff, "%s %s", train1, train2) ;
        //Envoie la chaine uartBuff sur le sevice TX UART BLE
        uartServicePtr->write(uartBuff, UARTService::BLE_UART_SERVICE_MAX_DATA_LEN);
        // Réinitialiser la chaîne uartBuff en entrant 0 dans les premiers caractères UART_BUFFER
        memset(uartBuff, 0, UART_BUFFER);
    } else {
        char laser1[1];
        char laser2[1];

        //Transformation des valeurs numeriques en chaine de caracteres
        float meas = photodiode1.read();
        sprintf(laser1,"%1.0f",meas);
        sprintf(laser2,"%1.0f",photodiode2);
        //Integre les trois chaines de caractere contenant les ts dans la chaine uartBuff
        sprintf(uartBuff, "%s %s", laser1, laser2) ;
        //Envoie la chaine uartBuff sur le sevice TX UART BLE
        uartServicePtr->write(uartBuff, UARTService::BLE_UART_SERVICE_MAX_DATA_LEN);
        // Réinitialiser la chaîne uartBuff en entrant 0 dans les premiers caractères UART_BUFFER
        memset(uartBuff, 0, UART_BUFFER);
    }


}

/*************************/
/* Connexion BLE réussie */
/*************************/
void BleConnectionCallback(const Gap::ConnectionCallbackParams_t *params)
{
    // Signal de connexion BLE: allume / éteint la LED avec une période de 1 seconde
    ConnectedBLE();
}

/*****************************/
/* Déconnexion du client BLE */
/*****************************/
void BleDisconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    (void)params;
    // Redémarrer la publicité
    Timer_Reset();
    BLE::Instance().gap().startAdvertising();


}

/***************************/
/* Rx de BLE et Rx vers USB*/
/***************************/
void BleOnDataWrittenCallback(const GattWriteCallbackParams *params)
{
    char reception[UART_BUFFER];
    char commande[3];
    if (params->handle == uartServicePtr->getTXCharacteristicHandle()) {
        // Copie de la chaine reçue dans reception
        sprintf(reception,"%s", params->data);
        // Copie dans la chaine commande des deux premier caracteres de la chaine reception
        sprintf(commande,"%c%c%c", reception[0], reception[1], reception[2]);

        if( strcmp(commande, "RST" )==0 ) {
            Timer_Reset();
            update();
        }
        if( strcmp(commande, "SOL" )==0 ) {
            gamemode = 'S';
            laser=1;
            update();
        }
        if( strcmp(commande, "DUO" )==0 ) {
            gamemode = 'D';
            laser=1;
            update();
        }
        if( strcmp(commande, "LAP" )==0 ) {
            gamemode = 'L';
            laser=1;
            update();
        }
        if( strcmp(commande, "MEN" )==0 ) {
            gamemode = 'M';
            laser=0;
            update();
        }
        if( strcmp(commande, "LAS" )==0 ) {
            gamemode = 'A';
            laser=1;
            update();
        }
        if( strcmp(commande, "BIP" )==0 ) {
            Timer_Reset();
            timer_player_1.start();
            timer_player_2.start();
            begin1 = timer_player_1.read_ms();
            begin1 = timer_player_2.read_ms();
            myled=1;
            wait(0.5);
            myled=0;
        }
    }
}

/***************************/
/* Erreur sur le canal BLE */
/***************************/

void onBleError(ble_error_t error)
{
    /* Entrer le traitement des erreurs */
}

/**************************************/
/* Initialisation du service BLE UART */
/**************************************/

void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {

        /* En cas d'erreur, transmettez le traitement d'erreur à onBleInitError*/
        onBleError(error);
        return;
    }

    /* Assurez-vous qu'il s'agit de l'instance par défaut de BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onConnection(BleConnectionCallback);
    ble.gap().onDisconnection(BleDisconnectionCallback);
    ble.gattServer().onDataWritten(BleOnDataWrittenCallback);

    /* Configuration du service primaire. */
    UARTService uartService(ble);
    uartServicePtr = &uartService;

    /* Configurer la publicité */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)UARTServiceUUID_reversed, sizeof(UARTServiceUUID_reversed));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(500); /* 500ms. */
    ble.gap().startAdvertising();

    // Attend les événements sur BLE
    while (true) {
        
        ble.waitForEvent();
        update();
        //RF Receive Code
        device_receive();
        wait(0.01);
    }
}

/********/
/* MAIN */
/********/
int main(void)
{
    device.baud(2400);
    myButton.fall(&photodiode_triggered);
    photodiode.fall(&photodiode_triggered);
    

    /****** START Initialiser BLE **********/
    BLE &ble = BLE::Instance();
    ble.init(bleInitComplete);
    /******* FIN initialise BLE ***********/
}

