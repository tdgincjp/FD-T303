
#include "stdafx.h"
#include <stdio.h>

#include "Language.h"
#include "..\\data\\DataFile.h"
#include "..\\DEFS\\constant.h"

//#pragma warning(disable: 4566)

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define MAX_LANGUAGE_TYPES		2
extern BOOL g_LanguageSelected;

const char *pbaDisplayText[][MAX_LANGUAGE_TYPES] = 
{
	{ "CORR"							, "CORR"				}, // IDX_CORR
	{ "OK"								, "OK"					}, // IDX_OK2
	{ "CANCEL"							, "CANCEL"				}, // IDX_CANCEL
	{ "CHQ"								, "CHQ"					}, // IDX_CHQ
	{ "SAV"								, "EP"					}, // IDX_SAV
	{ "CRD"								, "CRTE"				}, // IDX_CRD
	{ "SELECTED"						, "CHOISIE"				}, // IDX_SELECTED           
	{ "DO NOT"							, "NE PAS"				}, // IDX_DO_NOT
	{ "REMOVE CARD"						, "RETIRER LA CARTE"	}, // IDX_REMOVE_CARD        
	{ "SELECT"							, "CHOISIR"				}, // IDX_SELECT             
	{ " YES"							, " OUI"				}, // IDX_YES             
	{ " NO"								, " NON"				}, // IDX_NO              
	{ "APPROVED"						, "APPREUVE"			}, // IDX_EMV_APPROVED
	{ "DECLINED"						, "REFUSEE"				}, // IDX_EMV_DECLINED
	{ "ENTER PIN and OK"				, "ENTRER NIP et OK"	}, // IDX_EMV_ENTER_PIN
	{ "PIN OK"							, "NIP OK"				}, // IDX_EMV_PIN_OK
	{ "INCORRECT PIN"					, "NIP NON VALIDE"			}, // IDX_EMV_INCORRECT_PIN
	{ "CANCEL OR ENTER" 				, "CANCEL OU ENTRE"			}, // IDX_EMV_CANCEL_OR_ENTER
	{ "CARD ERROR"						, "ERREUR CARTE"			}, // IDX_EMV_CARD_ERROR
	{ "CHIP ERROR"						, "ERREUR PUCE"				}, // IDX_EMV_CHIP_ERROR
	{ "PLEASE WAIT"						, "SVP ATTENDRE"			}, // IDX_EMV_PLEASE_WAIT
	{ "PROCESSING"						, "TRAITEMENT"				}, // IDX_EMV_PROCESSING
	{ "ERROR"							, "ERREUR"				}, // IDX_EMV_ERROR
	{ "REMOVE CARD"						, "RETIRER LA CARTE"			}, // IDX_EMV_REMOVE_CARD
	{ "INSERT CARD"						, "INSERER CARTE"			}, // IDX_EMV_USE_CHIP_READER
	{ "SWIPE CARD"						, "GLISSEZ CARTE"			}, // IDX_EMV_USE_MAGSTRIPE
	{ "TRY AGAIN"						, "RESSAIE"				}, // IDX_EMV_TRY_AGAIN
	{ "-OK?"							, "-OK?"				}, // IDX_OK                 
	{ "SELECT ACCOUNT"					, "CHOISIR COMPTE"			}, // IDX_SELECT_ACCT     
	{ "CARD REMOVED"					, "CARTE RETIREE"			}, // IDX_CARD_REMOVED    

	{ "ERROR"							, "ERREUR"				}, // IDX_ERROR            
	{ "COMMUNICATIONS"					, "COMMUNICATIONS"				}, // IDX_COMMUNICATIONS
	{ "REVERSED"						, "RENVERSE"				}, // IDX_REVERSED
	{ "Connecting..."					, "Connection..."				}, // IDX_SOCKET_CONNECT
	{ "Sending..."						, "Envoi..."				}, // IDX_SOCKET_SENDING
	{ "Receiving..."					, "Reception..."				}, // IDX_SOCKET_RECEIVING
	{ "Processing..."					, "Traitement..."				}, // IDX_SOCKET_PROCESSING
	{ "FAILED"							, "ECHAPPE"				}, // IDX_SOCKET_NOTSETUP
	{ "Send Error"						, "Envoie Erreur"			}, // IDX_SOCKET_SENDING_ERROR
	{ "Successful"						, "Reussi"				}, // IDX_HOST_INIT_DONE
	{ "Recv Time Out"					, "Cnxn interrompu"		}, //IDX_SOCKET_RECV_TIMEOUT
	{ "Key Exchange Failed"				, "Echange de cle cryptee echoue"		}, //IDX_KEY_EXCHANGE_ERROR
	{ "SALE"							, "ACHAT"				}, // IDX_PURCHASE        
	{ "RETURN"							, "RETOUR"				}, // IDX_REFUND          
	{ "PRE-AUTH"						, "PREAUTORIS"				}, // IDX_PREAUTH         
	{ "VOID"							, "ANNUL"				}, // IDX_VOID
	{ "VOIDED"							, "ANNULE"				}, // IDX_VOIDED
	{ "COMPLETION"						, "ACHEVEMENT"				}, // IDX_COMPLETION
	{ "FORCE POST"						, "VENTE AUTONOM"			}, // IDX_FORCEPOST

	{ "ADMIN"							, "GESTION"				}, // IDX_ADMIN
	{ "OEM ID"							, "ID OEM"				}, // IDX_OEMID
	{ "Processor Number"				, "No. processor"			}, // IDX_PROCESSORS_NUMBER
	{ "Architecture"					, "Architecture"			}, // IDX_ARCHITECTURER
	{ "Processor Mask"					, "Masque de Processeur"		}, // IDX_PROCESSOR_MASK
	{ "OS test is being performed"		, "Test d'OS est en cours"		}, // IDX_OS_TEST_PERFORMED
	{ "Self-test is being performed"	, "Auto test en cours"			}, // IDX_SELF_TEST_PERFORMED
	{ "App Version"						, "Version App"				}, // IDX_VERSION_NUMBER
	{ "Initializing EMV."				, "Initialisation EMV."			}, // IDX_INIT_EMV

	{ "ADMIN MENU"						, "MENU GESTION"			}, // IDX_ADMIN_MENU
	{ "CONFIG MENU"						, "MENU CONFIG"				}, // IDX_CONFIG_MENU
	{ "REPORT MENU"						, "MENU RAPPORT"			}, // IDX_REPORT_MENU

	{ "REPRINT"							, "REIMPRIMER"				}, // IDX_REPRINT_MENU
	{ "LANGUAGE"						, "LANGUE"				}, // IDX_LANGUAGE_MENU
	{ "SOUND"							, "SON"					}, // IDX_SOUND_MENU
	{ "TRAINING"						, "FORMATION"				}, // IDX_TRAINING_MENU
	{ "COMMS TEST"						, "TEST COMM"				}, // IDX_COMMS_TEST_MENU
	{ "CLERK PSWD"						, "MOT DE PAS COMMIS"			}, // IDX_CLERK_PSWD_MENU
	{ "MNGR PSWD"						, "MOT DE PASSE GERANT"			}, // IDX_MNGR_PSWRDT_MENU
	{ "DOWNLOAD"						, "TELECHARGEMENT"			}, // IDX_DOWNLOAD_MENU
	{ "KEY EXCHANGE"					, "ECHANGE DE CLES"				}, // IDX_KEY_EXCHANGE_MENU
	{ "POSX DOWNLOAD"					, "POSX DOWNLOAD"			}, //IDX_POSX_DOWNLOAD_MENU

	{ "GENERAL PARMS"					, "PARMS GENERAUX"			}, // IDX_GENERAL_PARMS
	{ "TERM CONFIG"						, "CONFIG TERM"			}, // IDX_TERM_CONFIG
	{ "DIAL CONFIG"						, "CNFG COMPOSITION"			}, // IDX_DIAL_CONFIG
	{ "ETHERNET CNFG"					, "CNFG ETHERNET"			}, // IDX_ETHERNET_CNFG
	{ "PRINTER CNFG"					, "CNFG IMPRIMANTE"			}, // IDX_PRINTER_CNFG

	{ "TERM ID"							, "ID TERM"				}, // IDX_TERM_ID
	{ "COMM TYPE"						, "TYPE DE COMM"			}, // IDX_COMM_TYPE
	{ "CLERK ID"						, "ID COMMIS"				}, // IDX_CLERK_ID
	{ "FRAUD CHECK"						, "VERIF FRAUDE"		}, // IDX_FRAUD_CHECK
	{ "MANUAL ENTRY"					, "ENTREE MANUEL"			}, // IDX_MANUAL_ENTRY
	{ "TIP PROMPT"						, "INVITES POURBOIRE"			}, // IDX_TIP_PROMPT
	{ "INVOICE"							, "FRACTURE"				}, // IDX_INVOICE_MENU
	{ "SURCHARGE"						, "FRAIS OPERATION"			}, // IDX_SURCHARGE_MENU
	{ "CASHBACK"						, "REMISE"				}, // IDX_CASHBACK_MENU
	{ "PREAUTH EXPIRY"					, "JOURS DE PREAUTOR"				}, // IDX_PREAUTH_AGING_DAYS_MENU
	{ "DAYS"							, "JOURS"				}, // IDX_DAYS

	{ "PHONE #"							, "# TEL"				}, // IDX_PHONE_MENU
	{ "SEC PHONE#"						, "# TEL SEC"				}, // IDX_PHONE2_MENU
	{ "DIAL MODE"						, "MODE DE COMPS"			}, // IDX_DIAL_MODE_MENU
	{ "BAUD RATE"						, "VITESSE DE TRANS"		}, // IDX_BAUD_RATE_MENU

	{ "HOST ADDR"						, "ADDR D'HOTE"				}, // IDX_HOST_ADDR
	{ "HOST SETUP"						, "INSTALLER D'HOTE"				}, // IDX_HOST_SETUP
	{ "IP PORT"							, "PORT IP"				}, // IDX_IP_PORT
	{ "DIAL BACKUP"						, "SECOURS DE COMPS"			}, // IDX_DIAL_BACKUP
	{ "STATIC IP"						, "IP STATIQUE"			}, // IDX_STATIC_IP
	{ "HEADER MSG"						, "MESSAGE TETE"			}, // IDX_HEADER_MSG
	{ "TRAILER MSG"						, "MESSAGE DE FIN"			}, // IDX_TRAILER_MSG
	{ "COPY LOG"						, "COPY LOG"			}, // IDX_COPY_LOG			JC Nov 11/15 Add French
	{ "RESTART"							, "REDEMARRAGE"			}, // IDX_RESTART			JC Nov 11/15 Add French
	{ "SHOW MORE"						, "MONTRE PLUS"			},//IDX_SHOW_MORE			JC Nov 11/15 Add French
	{ "GO BACK"							, "RETOURNER"			},//IDX_GO_BACK					JC Nov 11/15 Add French
	{ "DEBUG API"						, "DEBOGAGE API"			},//IDX_DEBUG_API			JC Nov 11/15 ADD DEBUG API
	{ "ENABLE"							, "ACTIVER"					},//IDX_ENABLE			JC Nov 11/15 ADD DEBUG API
	{ "DISABLE"							, "DISABLE"					},//IDX_DISABLE			JC Nov 11/15 ADD DEBUG API
	{ "BATCH TOTALS"					, "TOTAUX DE LOT"			}, // IDX_BATCH_TOTALS
	{ "BATCH CLOSE"						, "FERMETURE DE LOT"			}, // IDX_BATCH_CLOSE
	{ "INVOICE DET"						, "DET FACTURE"				}, // IDX_INVOICE_DET
	{ "CARD DETAIL"						, "DET CARTE"				}, // IDX_CARD_DETAIL
	{ "CARD SUMMARY"					, "SOMMAIRE CARTE"			}, // IDX_CARD_SUMMARY
	{ "DEBIT SUMMARY"					, "SOMMAIRE DEBIT"			}, // IDX_DEBIT_SUMMARY
	{ "CLERK DETAIL"					, "DETAIL COMMIS"			}, // IDX_CLERK_DETAIL
	{ "TIP DETAIL"						, "DETAIL POURBOIRE"			}, // IDX_TIP_DETAIL
	{ "PRE-AUTH"						, "PRE-AUTH"				}, // IDX_PRE_AUTH_MENU
	{ "EMV INFO"						, "INFO EMV"				}, // IDX_EMV_INFO

	{ "AID PARAMS"						, "PARMS AID"				}, // IDX_AID_PARAMS
	{ "CAPK INFO"						, "INFO CAPK"				}, // IDX_CAPK_INFO_MENU
	{ "STATISTIC INFO"					, "INFOS STATISTIQUES"		}, // IDX_EMV_STATISTIC_MENU
	
	{ "OFFLINE INFO"					, "INFO DECONNECTE"			}, // IDX_OFFLINE_INFO

	{ "  CHEQUE"						, "  CHEQUE"				}, // IDX_CHEQUE
	{ "  SAVING"						, "  EPARGNE"				}, // IDX_SAVING
	{ "Time Out"						, "DELAI DEPASSE"			}, // IDX_TIME_OUT
	{ "Enter Password"					, "ENTRE MOT DE PASSE"			}, // IDX_ENTER_PASSWORD
	{ "Enter Amount"					, "ENTRE MONTANT"			}, // IDX_ENTER_AMOUNT
	{ "Enter Clerk"						, "ENTRE COMMIS"			}, // IDX_ENTER_CLERK
	{ "Id"								, "ID"					}, // IDX_ID_NAME
	{ "Enter Receipt"					, "ENTRE RECU"				}, // IDX_ENTER_RECEIPT
	{ "Number"							, "Numero"				}, // IDX_NUMBER_NAME
	{ "Insert/Tap"						, "Inserer,Tapper"		}, // IDX_INSERT_SWIPE_TAP
	{ "Swipe / Tap"						, "Glisser,Tapper"		}, // IDX_SWIPE_TAP
	{ "Insert"							, "Inserer"				}, // IDX_INSERT_SWIPE
	{ "Swipe"							, "Glisser"				}, // IDX_SWIPE
	{ "or Key Card #"					, "ou Entrer manuellement"		}, // IDX_KEY_CARD_NUMBER
	{ "Key Card #"						, "Entrer manuellement"		}, // IDX_KEY_CARD_NUMBER1
	{ "Reader Open Failed"				, "Echec Lecteur Cartes"			}, // IDX_FAIL_OPEN_DEVICE
	{ "Pass to"							, "passer au"				}, // IDX_PASS_TO
	{ "Customer"						, "Client"				}, // IDX_CUSTOMER
	{ "Clerk"							, "Commis"				}, // IDX_CLERK
	{ "Enter Cashback"					, "Entrer Remise"			}, // IDX_ENTER_CASHBACK
	{ "Amount"							, "Montant"				}, // IDX_AMOUNT
	{ "PLEASE WAIT"						, "UN MOMENT SVP"			}, // IDX_PLEASE_WAIT
	{ "Fee"								, "Frais"				}, // IDX_FEE
	{ "APPROVED"						, "APPROUVEE"				}, // IDX_APPROVED  
	{ "PARTIAL APPROVED"				, "APPROB PARTIELLE"				}, // IDX_PARTIAL_APPROVED  
	{ "AMOUNT DUE"						, "MONTANT DU"				}, // IDX_AMOUNT_DUE  
	{ "AVAIL BAL"						, "SOLDE DISPON"				}, // IDX_AVAIL_BAL  
	{ "Unsuccessful" 					, "Unsuccessful"			}, // IDX_UNSUCCESSFUL  
	{ "Select Account"					, "Choisir Compte"			}, // IDX_SELECT_ACCOUNT        
	{ "Retrieve Card"					, "Recuperer la Carte"			}, // IDX_RETRIEVE_CARD  
	{ "Pass to clerk"					, "Passer a Commis"			}, // IDX_PASS_TO_CLERK        
	{ "AUTH"							, "AUTH"				}, // IDX_AUTH_CODE 
	{ "Inv.#"							, "Fac#"				}, // IDX_INVO_NUMBER   
	{ "Declined"						, "Refusee"				}, // IDX_DECLINED   
	{ "Press OK for"					, "Appuyer OK"				}, // IDX_PRESS_OK_FOR   
	{ "CUSTOMER COPY"					, "COPIE CLIENT"			}, // IDX_CUSTOMER_COPY   
	{ "Enter Expiry"					, "Entrer Expiration"			}, // IDX_ENTER_EXPIRY   
	{ "Date"							, "Date"				}, // IDX_DATE   
	{ "Imprint Card"					, "Imprimer Carte"			}, // IDX_IMPRINT_CARD   
	{ "Press OK"						, "Appuyer OK"				}, // IDX_PRESS_OK   
	{ "Do not remove"					, "Ne pas retirer"			}, // IDX_DO_NOT_REMOVE   
	{ "Card!"							, "Carte!"				}, // IDX_CARD  
	{ "Please"							, "SVP"					}, // IDX_PLEASE  
	{ "Enter Last"						, "Entrer Dernier"			}, // IDX_ENTER_LAST  
	{ "Digits"							, "Chiffres"				}, // IDX_DIGITS  
	{ "TRANSACTION"						, "OPERATION"				}, // IDX_TRANSACTION  
	{ "Cancelled"						, "Annule"				}, // IDX_CANCELLED  
	{ "Cancel"							, "Cancel"				}, // IDX_CANCELL  
	{ "This CARD "						, "Cette Carte"				}, // IDX_CARD_NOT_SUPPORT  

	{ "DUPLICATE"						, "DUPLICATA"				}, // IDX_DUPLICATE  
	{ "TERM #"							, "TERM #"				}, // IDX_RECEIPT_TERM  
	{ "MERCH #"							, "MERCH #"				}, // IDX_RECEIPT_MERCH  
	{ "INVOICE#"						, "FACTURE"				}, // IDX_RECEIPT_INVOICE 
	{ "RECEIPT#"						, "FACTURE"				}, // IDX_RECEIPT_RECEIPTNUM  
	{ "CLERK"							, "COMMIS"				}, // IDX_RECEIPT_CLERK  
	{ "SEQ#"							, "SEQ#"				}, // IDX_RECEIPT_TRANS
	{ "AUTH#"							, "NO AUTO"				}, // IDX_RECEIPT_AUTH  
	{ "B"								, "L"					}, // IDX_RECEIPT_BATCH_ONE  

	{ "CARD"							, "CARTE"				}, // IDX_RECEIPT_CARD  
	{ "CARD TYPE"						, "CARTE"				}, // IDX_RECEIPT_CARDTYPE  
	{ "CREDIT"							, "CREDIT"				}, // IDX_RECEIPT_CREDIT  
	{ "DEBIT"							, "DEBIT"				}, // IDX_RECEIPT_DEBIT  
	{ "DATE"							, "DATE"				}, // IDX_RECEIPT_DATE  
	{ "TIME"							, "HEURE"				}, // IDX_RECEIPT_TIME
	{ "RECEIPT"							, "RECU"				}, // IDX_RECEIPT_RECEIPT  
	{ "REFERENCE"						, "REFERENCE"				}, // IDX_RECEIPT_REFERENCE  

	{ "SALE"							, "ACHAT"				}, // IDX_RECEIPT_SALE  
	{ "SALE VOID"						, "VENTE ANNUL"				}, // IDX_RECEIPT_SALE_VOID  
	{ "REFUND"							, "RETOUR"				}, // IDX_RECEIPT_REFUND  
	{ "REFUND VOID"						, "RETOUR ANUL"				}, // IDX_RECEIPT_REFUND_VOID
	{ "PREAUTH"							, "PREAUTOR"				}, // IDX_RECEIPT_PREAUTH  
	{ "COMPLETION"						, "COMPLETION"				}, // IDX_RECEIPT_COMPLETION  
	{ "FORCE POST"						, "VENTE AUTONUM"			}, // IDX_RECEIPT_FORCE_POST  

	{ "AMOUNT"							, "MONTANT"				}, // IDX_RECEIPT_AMOUNT  
	{ "CASHBACK"						, "REMISE"				}, // IDX_RECEIPT_CASHBACK
	{ "CB FEE"							, "FRAIS DE RETRAITE"				}, // IDX_RECEIPT_CASHBACK_FEE
	{ "TIP"								, "POURBOIRE"				}, // IDX_RECEIPT_TIP  
	{ "SURCHARGE"						, "FRAIS"				}, // IDX_RECEIPT_SURCHANGE  
	{ "TOTAL"							, "TOTAL"				}, // IDX_RECEIPT_TOTAL  

	{ "CHIP CARD SWIPED"				, "CARTE A PUCE GLISSEE"		}, // IDX_RECEIPT_CHIP_CARD_SWIPED  
	{ "CHIP CARD KEYED"					, "CARTE A PUCE TAPEE"			}, // IDX_RECEIPT_CHIP_CARD_KEYED  
	{ "TRANSACTION"						, "OPERATION"				}, // IDX_RECEIPT_TRANSACTION
	{ "APPROVED"						, "APPROUVEE"				}, // IDX_RECEIPT_APPROVED  
	{ "NOT COMPLETED"					, "NON COMPLETEE"			}, // IDX_RECEIPT_NOT_COMPLETED  
	{ "NOT APPROVED"					, "REFUSEE"				}, // IDX_RECEIPT_NOT_APPROVED  

	{ "VERIFIED BY PIN"					, "VERIFIEE PAR NIP"			}, // IDX_RECEIPT_VERIFIED_BY_PIN
	{ "SIGNATURE NOT REQUIRED"			, "AUCUNE SIGNATURE NECESSAIRE"			}, // IDX_SIGNATURE_NOT_REQUIRED
	{ "MERCHANT COPY"					, "COPIE COMMERCANT"			}, // IDX_RECEIPT_MERCHANT_COPY  
	{ "CUSTOMER COPY"					, "COPIE CLIENT"			}, // IDX_RECEIPT_CUSTOMER_COPY  
	{ "CANCELLED"						, "ANNULEE"				}, // IDX_RECEIPT_CANCELLED  
	{ "CHIP CARD MALFUNCTION"			, "DEFAILLANCE CARTE A PUCE"		}, // IDX_RECEIPT_CHIP_CARD_MALFUNCTION  
	{ "I AGREE TO PAY THE ABOVE"		, "JE CONSENS A PAYER CE"				}, // IDX_RECEIPT_THANK_YOU1  
	{ "TOTAL AMOUNT ACCORDING"			, "TOTALCONFORMEMENT A L’"			}, // IDX_RECEIPT_THANK_YOU2  
	{ "TO CARD ISSUER AGREEMENT"		, "ENTENTE DE L’EMETTEUR"		}, // IDX_RECEIPT_THANK_YOU3  
	{ "(MERCHANT AGREEMENT IF"			, "(ENTENTE MARCHAND SI"		}, // IDX_RECEIPT_THANK_YOU4  
	{ "CREDIT VOUCHER)"					, "NOTE CREDIT)"		}, // IDX_RECEIPT_THANK_YOU5  

	{ "Retain this copy for statement ", "JE CONSENS A PAYER CE"	}, // IDX_RECEIPT_THANK_YOU21  
	{ "validation"				, "TOTALCONFORMEMENT A L’"	}, // IDX_RECEIPT_THANK_YOU22  
	{ "TERMS AND CONDITIONS: No refunds "		, "ENTENTE DE L’EMETTEUR"	}, // IDX_RECEIPT_THANK_YOU23  
	{ "For further inquiry call us at "			, "(ENTENTE MARCHAND SI"	}, // IDX_RECEIPT_THANK_YOU24  
	{ "1-888-123-4567"							, "NOTE CREDIT)"			}, // IDX_RECEIPT_THANK_YOU25  

	{ "CARDHOLDER SIGNATURE"			, "SIGNATURE DU TITULAIRE"		}, // IDX_RECEIPT_CARDHOLDER_SIGNATURE  

	{ "By Entering a verified"			, "EN ENTRANT UN NIP VERIFIE,"				}, // IDX_RECEIPT_THANK_YOU11  
	{ "PIN,Cardholder agrees to"		, "DETENTEUR CONSENT A PAYER"			}, // IDX_RECEIPT_THANK_YOU12  
	{ "pay issuer such total in"		, "A EMETTEUR UN TEL TOTAL EN"		}, // IDX_RECEIPT_THANK_YOU13  
	{ "accordance with issuer's"		, "ACCORD AVEC L’ENTENTE DE L"		}, // IDX_RECEIPT_THANK_YOU14  
	{ "agreement with cardholder"		, "EMETTEUR DETENTEUR DE CARTE"		}, // IDX_RECEIPT_THANK_YOU15  

	{ "HOST OPEN BATCH TOTALS"			, "TOTAUX LOT ACTUEL HOTE"		}, // IDX_HOST_OPEN_BATCH_TOTALS  
	{ "HOST BATCH TOTALS"				, "TOTAUX DU SERVEUR"			}, // IDX_HOST_BATCH_TOTALS  
	{ "STORE BATCH TOTALS"				, "TOTAUX DU MAGASIN"			}, // IDX_STORE_BATCH_TOTALS  
	{ "END OF REPORT"					, "FIN DU RELEVE"			}, // IDX_END_OF_REPORT  

	{ "BATCH"							, "LOT"					}, // IDX_RECEIPT_BATCH  
	{ "OPEN"							, "OUVERTURE"				}, // IDX_RECEIPT_OPEN  
	{ "CLOSE"							, "FERMETURE"				}, // IDX_RECEIPT_CLOSE  
	{ "BATCH IN BALANCE"				, "LOT EN SOLDE"			}, // IDX_BATCH_IN_BALANCE  
	{ "BATCH OUT OF BALANCE"			, "HORS DE SOLDE"			}, // IDX_BATCH_OUT_OF_BALANCE  

	{ "BATCH CLOSED"					, "LOT FERME"				}, // IDX_RECEIPT_BATCH_CLOSED  
	{ "BATCH NOT CLOSED"				, "LOT NON FERME"			}, // IDX_RECEIPT_BATCH_NOT_CLOSED  
	{ "BATCH CLOSE TOTALS"				, "TOTAUX FERMATURE DE LOT"		}, // IDX_RECEIPT_BATCH_CLOSE_TOTALS  

	{ "RETURN"							, "RETOUR"				}, // IDX_RECEIPT_RETURN  
	{ "VOID"							, "ANNUL"				}, // IDX_RECEIPT_VOID  
	{ "NET TOT" 						, "TOT NET"				}, // IDX_RECEIPT_NET_TOT  

	{ "INVOICE DETAIL REPORT"			, "RAPPORT DETAIL DE FACTURE"		}, // IDX_INVOICE_DETAIL_REPORT  

	{ "SURCHARGE"						, "FRAIS"				}, // IDX_RECEIPT_SURCHARGE  
	{ "CARD DETAIL REPORT"				, "DETAIL CARTE RELEVE"			}, // IDX_CARD_DETAIL_REPORT  

	{ "CARD TOTALS"						, "TOTAUX DU CARTE"			}, // IDX_RECEIPT_CARD_TOTALS  
	{ "SUB"								, "SUB"					}, // IDX_RECEIPT_SUB  
	{ "TXN"								, "TRN"					}, // IDX_RECEIPT_TXN  
	{ "ALL TRANSACTIONS"				, "TRANSACTIONS"			}, // IDX_RECEIPT_ALL_TRANSACTIONS  
	{ "COMPL"							, "COMPLETE"				}, // IDX_RECEIPT_COMPL  
	{ "CARD SUMMARY REPORT"				, "SOMMAIRE CARTE RELEVE"		}, //   IDX_CARD_SUMMARY_REPORT

	{ "SURCH"							, "FRAIS SUP"				}, // IDX_RECEIPT_SURCH  
	{ "DEBIT SUMMARY REPORT"			, "SOMMAIRE DEBIT RELEVE"		}, //   IDX_DEBIT_SUMMARY_REPORT

	{ "CLERK"							, "COMMIS"				}, // IDX_RECEIPT_REPORT_CLERK  
	{ "REPORT"							, "RELEVE"				}, //   IDX_RECEIPT_REPORT
	{ "ALL OPERATORS REPORT"			, "LISTE DES OPER RELEVE"		}, // IDX_ALL_OPERATORS_REPORT  
	{ "INV"								, "FACT"				}, //   IDX_RECEIPT_INV
	{ "NONE"							, "AUCUN"				}, // IDX_RECEIPT_NONE  
	{ "TIPS"							, "POURBOIRES"				}, //   IDX_RECEIPT_TIPS
	{ "CLERK ID"						, "COMMIS"				}, //   IDX_RECEIPT_CLERK_ID

	{ "TIP REPORT"						, "POUBOIRE RELEVE"			}, //   IDX_TIP_REPORT
	{ "PREAUTH REPORT"					, "PREAUTOR RELEVE"			}, //   IDX_PREAUTH_REPORT

	{"Press OK to confirm"				, "Appuyez OK pour confirmer"		}, //IDX_PRESS_OK_CONFIRM
	{"Press OK to"						, "Appuyez OK pour"			}, //IDX_PRESS_OK_TO
	{ "BATCH CLOSE"						, "FERMER LOT"				}, // IDX_CLOSE_BATCH
	{ "Printing"						, "Imprime"				}, // IDX_PRINTING
	{ "Change"							, "Changer"				}, // IDX_CHANGE
	{ "Confirmed"						, "Confirme"				}, // IDX_CONFIRMED
	{ "Confirm!"						, "Confirm!"				}, // IDX_CONFIRM
	{ "Confirm New"						, "Confirme Nouveau"			}, // IDX_CONFIRM_NEW
	{ "   Prev"							, "   Pred"				}, // IDX_PREV
	{ "   Next"							, "   Proch"				}, // IDX_NEXT
	{ "Enter Invoice"					, "Entrer Facture"			}, // IDX_ENTER_INVOICE
	{ "Void inv."						, "Inv Annul"				}, // IDX_VOID_INV
	{ "Enter Fee"						, "Entree Frais"			}, // IDX_ENTER_FEE
	{ "Enter New"						, "Entrer Nouveau"			}, // IDX_ENTER_NEW
	{ "Host Address"					, "Adresse Nouveau"			}, // IDX_HOST_ADDRESS
	{ "Password"						, "Mot de Passe"			}, // IDX_PASSWORD
	{ "Phone Number"					, "Numero de Telephone"			}, // IDX_PHONE_NUMBER
	{ "No Network!"						, "Aucun Reseau!"			}, // IDX_NO_NETWORK
	{ " Mismatched"						, "Inadequation"			}, // IDX_MISMATCHED
	{ " Dup Invoice#"					, " Dup Facture#"			}, // IDX_DUP_INVOICE
	{ " ReEnter"						, "Re-entrer"				}, // IDX_RE_ENTER
	{ " Enter"							, "Entrer"				}, // IDX_ENTER

	{ "INVOICE"							, "FACTURE"				}, // IDX_INVOICE
	{ "MODE"							, "MODE"				}, // IDX_MODE
	{ "Enter Tip"						, "Entrer Pourboire"			}, // IDX_ENTER_TIP
	{ "LOCKED"							, "LOCKED"				}, // IDX_LOCKED

	{ "Locked - Enter"					, "Expire - Entrez"			}, // IDX_LOCKED_ENTERD
	{ "Super Password"					, "Mot de Passe Super"			}, // IDX_SUPER_PASSWORD
	{ "Manager Password"				, "Mot de Passe Gerant"			}, // IDX_MANAGER_PASSWORD
	{ "CLERK PW"						, "Mot de Passe Commis"			}, // IDX_CLERK_PW
	{ "MANAGER PW"						, "Mot de Passe Gerant"			}, // IDX_MANAGER_PW

	{ "Non-Initialized"					, "Non Initialise"			}, // IDX_NON_INITIALIZED
	{ "Host Download" 					, "Telechargement d'hote"		}, // IDX_HOST_DOWNLOAD
	{ "Required"						, "Requis"				}, // IDX_REQUIRED

	{ "EMV AID"							, "EMV AID"				}, // IDX_EMV_AID
	{ "EMV CAPK"						, "EMV CAPK"				}, // IDX_EMV_CAPK
	{ "Terminal ID"						, "ID Terminal"				}, // IDX_TERMINAL_ID
	{ "OPTION"							, "OPTION"				}, // IDX_OPTION

	{ "First Tip"						, "Premier Pourboire"			}, // IDX_FIRST_TIP
	{ "Second Tip"						, "Deuxieme Pourboire"			}, // IDX_SECOND_TIP
	{ "Third Tip"						, "Troisieme Pourboire"			}, // IDX_THIRD_TIP
	{ "Percent"							, "Pourcent"				}, // IDX_PERCENT

	{ "Exiting"							, "Sotie"				}, // IDX_EXITING
	{ "Training Mode"					, "Mode Formation"			}, // IDX_TRAINING_MODE
	{ "Initializing..."					, "Initialisation..."			}, // IDX_INITIALIZING
	{ "Entering"						, "Entrant"				}, // IDX_ENTERING
	{ "No Txns to Void"					, "Aucune Trxns d'anulle"		}, // IDX_NO_RECORDING_LOG
	{ "Card Does Not Match"				, "Carte Ne Correspond Pas"		}, // IDX_CARD_NOT_MATCH
	{ "Approval Code"					, "Code Apporbation"			}, // IDX_APPROVAL_CODE
	{ "DEBIT NOT SUPPORTED"				, "DEBIT N'EST PAS SUPPORTE"		}, // IDX_DEBIT_NOT_SUPPORT
	{ "Can't Print"						, "Ne Peut Pas Imprimer"		}, // IDX_CAN_NOT_PRINT
	{ "swipe chip card" 				, "glissez carte de puce"		}, // IDX_CANNOT_SWIPE_CHIP
	{ "DECLINED BY CARD"				, "REFUSEE PAR CARTE"			}, // IDX_DECLINED_BY_CARD

	{ "CARD BLOCKED"					, "CART BLOQUEE"			}, // IDX_CARD_BLOCKED
	{ "NOT SUPPORTED"					, "NOT SUPPORTE"			}, // IDX_NO_SUPPORTED
	{ "APPLICATION"						, "APPLICATION"				}, // IDX_APPLICATION
	{ "Fraud Card"						, "Carte Fraudulent"			}, // IDX_FRAUD_CARD
	{ "BLOCKED"							, "BLOQUE"				}, // IDX_BLOCKED
	{ "NOT ACCEPTED"					, "NON ACCEPTE"				}, // IDX_NOT_ACCEPTED
	{ "CARD PROCESS"					, "PROCESS CARTE"			}, // IDX_CARD_PROCESS
	{ "Swipe Card" 						, "GLISSEE LA CARTE"			}, // IDX_FALLBACK_TO_MAGSTRIPE
	{ "Please Remove Card"				, "SVP Retirer Carte"			}, // IDX_PLEASE_REMOVE_CARD
	{ "Remove Card Quickly"				, "Retirer Carte rapidement"			}, // IDX_REMOVE_CARD_QUICKLY
	{ "DEBIT CARD"						, "CARTE DEBIT"				}, // IDX_DEBIT_CARD
	{ "CREDIT CARD"						, "CARTE CREDIT"				}, // IDX_CREDIT_CARD
	{ "MSR Open Fail"					, "MSR Ouverte Echoue"			}, // IDX_MSR_OPEN_FAIL
	{ "NEED DEBIT CARD" 				, "BESOIN CARTE DE DEBIT"		}, // IDX_EEED_DEBIT_CARD

	{ "No Preauth Txns"					, "Aucune Transaction Auth Pre"		}, // IDX_NO_PREAUTH_RECORDING

	{ "  Yes"							, "  Oui"				}, // IDX_BT_YES
	{ "  No"							, "  Non"				}, // IDX_BT_NO
	{ "VISA"							, "VISA"				}, // IDX_VISA
	{ "MASTERCARD"						, "MASTERCARD"				}, // IDX_MASTERCARD
	{ "AMERICAN EXPRESS"				, "AMERICAN EXPRESS"			}, // IDX_AMERICAN_EXPRESS
	{ "DISCOVER"						, "DISCOVER"				}, // IDX_DISCOVER
	{ "JCB"								, "JCB"					}, // IDX_JCB
	{ "DINERS CLUB"						, "DINERS CLUB"				}, // IDX_DINERS_CLUB
	{ "PROPRIETARY DEBIT"				, "DEBIT PROPRIETAIRE"			}, // IDX_PROPRIETARY_DEBIT

	{ "  Manual"						, "  Manuel"				}, // IDX_MANUAL
	{ "  Swiped"						, "  Glisser"				}, // IDX_SWIPED
	{ "  Chip"							, "  Puce"				}, // IDX_CHIP
	{ "  Ctls"							, "  Ctls"				}, // IDX_CTLS
	{ "Port"							, "Port"				}, // IDX_PORT
	{ "IP Port Number"					, "Numero de Port IP"			}, // IDX_IP_PORT_NUMBER
	{ "Unknown"							, "Inconnu"				}, // IDX_UNKNOWN
	{ "   ADMIN"						, "   ADMIN"				}, // IDX_BT_ADMIN
	{ "   FINAN"						, "   FINAN"				}, // IDX_BT_FINAN
	{ "Enter Online Pin"				, "Entre NIP en Ligne"			}, // IDX_ENTER_ONLINE_PIN

	{ "EMV Kernel ID:"					, "EMV Kernel ID:"			}, // IDX_EMV_KERNEL_ID
	{ "Terminal Type:"					, "Type de Terminal:"			}, // IDX_TERMINAL_TYPE
	{ "Terminal Capabilities:"			, "Capacites des Terminaux:"		}, // IDX_TERMINAL_CAPABILITIES
	{ "Add Term Capabilities:"			, "Capacites des Terminaux Suppl:"	}, // IDX_ADD_TERM_CAPABILITIES
	{ "Transaction Currency Code:"		, "Code de la Devise de Transaction:"	}, // IDX_TRANSACTION_CURRENCY_CODE
	{ "AID Name:"						, "Nom de l'AID:"			}, // IDX_AID_NAME
	{ "AID:"							, "AID:"				}, // IDX_AID
	{ "App Ver No:"						, "App Ver No:"				}, // IDX_APP_VER_NO
	{ "Floor Limit:"					, "Limit de Plancher:" 			}, // IDX_FLOOR_LIMIT
	{ "Threshold:"						, "Seul:"				}, // IDX_THRESHOLD
	{ "Targ,Percent:"					, "Pour Cent Cible:"			}, // IDX_TARG_PERCENT
	{ "Max Targ Percent:"				, "Pour Cent Cible Max:"		}, // IDX_MAX_TARG_PERCENT
	{ "Default TDOL:"					, "Defaulte TDOL:"			}, // IDX_DEFAULT_TDOL
	{ "Default DDOL:"					, "Defaulte DDOL:"			}, // IDX_DEFAULT_DDOL
	{ "TAC Default:"					, "Defaulte TAC:"			}, // IDX_TAC_DEFAULT
	{ "TAC Denial:"						, "TAC Denegation:"			}, // IDX_TAC_DENIAL
	{ "TAC Online:"						, "TAC Enligne:"			}, // IDX_TAC_ONLINE
	{ "Fallback Allowed"				, "Repli Permis"			}, // IDX_FALLBACK_ALLOWED
	{ "EMV CAPK REPORT"					, "RAPPORT EMV CAPK"			}, // IDX_EMV_CAPK_REPORT
	{ "EMV AID REPORT"					, "RAPPORT EMV AID"			}, // IDX_EMV_AID_REPORT
	{ "EMV STATISTIC REPORT"			, "RAPPORT EMV STATISTIC"			}, // IDX_EMV_STATISTIC_REPORT
	
	{ "RID"								, "RID"					}, // IDX_RID
	{ "Index"							, "Index"				}, // IDX_INDEX

	{ "AMOUNT ABOVE"					, "Montant dessus"				}, // IDX_AMOUNT_ABOVE
	{ "MAXIMUM"							, "MAXIMUM"				}, // IDX_MAXIMUM
	{ "Enter Maximum"					, "Entrez Maximum"				}, // IDX_ENTER_MAXIMUM
	{ "Enter Limit"						, "Entrez Limite"				}, // IDX_ENTER_LIMIT
	{ "RFID ERROR"						, "RFID ERREUR"				}, // IDX_RFID_ERROR
	{ "Invalid expiry date"				, "Invalid Date d'expiration"				}, // IDX_INVALID_EXPIRY_DATE
	{ "APPNAME"							, "APPNOM"				}, // IDX_RECOMMENDED_APPNAME

	{ "CONTACTLESS PARAMETERS"			, "paramètres sans contact"				}, // IDX_RCONTACTLESS_PARAMETERS

	{ "EmvAppVersion"					, "VersionEmvApp"				}, // IDX_EMV_APP_VERSION
	{ "MsdAppVersion"					, "VersionMsdApp"				}, // IDX_MSD_APP_VERSION
	{ "CVM Limit"						, "CVM Limite"				}, // IDX_CVM_LIMIT
	{ "TXN Limit"						, "TXN Limite"				}, // IDX_TXN_LIMIT
	{ "TOS"								, "TOS"				}, // IDX_TOS
	{ "RRL"								, "RRL"				}, // IDX_RRL
	{ "TTI"								, "TTI"				}, // IDX_TTI
	{ "TTQ"								, "TTQ"				}, // IDX_TTQ
	{ "MTI"								, "MTI"				}, // IDX_MTI
	

	{ "KEY MODULUS"						, "MODULE CL?"				}, // IDX_KEY_MODULUS
	{ "KEY EXPONENT"					, "EXPONENT CL?"				}, // IDX_KEY_EXPONENT
	{ "BATCH IS EMPTY"					, "LOT VIDE"				}, // IDX_BATCH_IS_EMPTY
	{ "Print Receipt?"					, "Imprimer recu?"				}, // IDX_DO_PRINT_RECEIPT

	{ "Receipt Type?"					, "Sorte de Recu?"				}, // IDX_RECEIPT_TYPE
	{ "CUST"							, "CLIENT"				}, // IDX_CUST
	{ "MERCH"							, "VENDEUR"				}, // IDX_MERCH  
	{ "SETTLE TYPE"						, "TYPE DE REGMNT"				}, // IDX_SETTLE_TYPE_MENU
	{ "CLEAR BATCH"						, "EFFACER LOT"				}, // IDX_CLEAR_BATCH_MENU
	{ "WARNING: Clear"					, "ATTENTION: Effacer"				}, // IDX_WARNING_CLEAR
	{ "Current Batch"					, "Lot Actuel"				}, // IDX_CURRENT_BATCH
	
	{ "Invoice Number: "				, "Numero de facture: "				}, // IDX_INVOICE_NUMBER
	{ "is not found"					, "n'est pas trouv?"				}, // IDX_IS_NOT_FOUND
	
	{ "Enter Original"					, "Entrer l'original"				}, // IDX_ENTER_ORIGINAL

	{ "Chip Card Read Fail"				, "lecture de la puce echouee"				}, // IDX_CHIP_CARD_READ_FAIL
	{ "PIN Entry Error"					, "Erreur: Code de NIP"				}, // IDX_PIN_ENTRY_ERROR
	{ "Offline Funds Decline"			, "Defaut de provision"				}, // IDX_OFFLINE_FUNDS_DECLINE
	{ "Technical Fallback"				, "Glisser la piste magnetique"				}, // IDX_TECHNICAL_FALLBACK

	{ "Invalid Card Number"				, "NO CARTE NON VALIDE"				}, // IDX_INVALID_CARD_NUMBER

	{ "OFFLINE DECLINED TRAN"			, "DECONNECT? TRANSACTION REFUS?"				}, // IDX_OFFLINE_DECLINED_TRAN
	{ "PAN:"							, "PAN:"				}, // IDX_PAN
	{ "PAN SEQ:"						, "PAN SEQ:"				}, // IDX_PAN_SEQ

	{ "IAC Default"						, "IAC defaut"				}, // IDX_IAC_DEFAULT
	{ "IAC Denial"						, "IAC refusee"				}, // IDX_IAC_DENIAL
	{ "IAC Online"						, "IAC en ligne"				}, // IDX_IAC_ONLINE
	{ "DE55"							, "DE55"				}, // IDX_DE55
	
	{ "Contactless Trans"				, "Transaction Sans Contact"				}, // IDX_CONTACTLESS_TRANS
	{ "Limit exceeded"					, "Limite depassee"				}, // IDX_LIMIT_EXCEEDED
	{ "Insert card"				, "Inserer carte"				}, // IDX_INSERT_SWIPE_CARD
	{ "Entry Mode:"						, "Mode d'entree"				}, // IDX_ENTRY_MODE
	{ "TAP FAILED. PLEASE"				, "Paiement sans contact echou?"				}, // IDX_TAP_FAILED_PLEASE
	{ "See Phone"				, "voir Telephone"				}, // IDX_SEE_PHONE
	{ "LAST PIN TRY"					, "Mauvais NIP: dernier essai"				}, // IDX_LAST_PIN_TRY
	{ "Select language"					, "Choisir langue"				}, // IDX_SELECT_LANGUAGE

	{ "IP Addr"							, "Addr IP"				}, // IDX_IP_ADDR
	{ "Mask"							, "Masq"				}, // IDX_IP_MASK
	{ "Gateway"							, "Porte"				}, // IDX_GATEWAY
	{ "DNS"								, "DNS"				}, // IDX_DNS
	{ "SOFT KEYBOARD"					, "Clavier logiciel"				}, // IDX_SOFT_KEYBOARD
	{ "Please Insert"					, "SVP Inserez"				}, // IDX_PLEASE_INSERT
	{ "CHIP Card"						, "Votre Puce"				}, // IDX_CHIP_CARD
	{ "Enter Threshold"					, "Entrez Seuil" },  //IDX_SURC_THRESHOLD 

	{ "CHEQUING"						, "CHEQUES"				}, // IDX_CHEQUING
	{ "SAVINGS"							, "EPARGNE"				}, // IDX_SAVINGS
	{ "DEFAULT"							, "Defaulte"				}, // IDX_DEFAULT

	{ "Enter Flex Settle"				, "Entrez L’heure"				}, // IDX_ENTER_SETTLE
	{ "Time(HH:MM)"						, "de Reglement"				}, // IDX_FLEX_TIME

	{ "Line"							, "Ligne"				}, // IDX_LINE
	{ "Header Line"						, "Ligne en-tete"				}, // IDX_HEADER_LINE
	{ "Trailer Line"					, "Ligne Bas de Page"				}, // IDX_TRAILER_LINE

	{ "POSX PARAM",						"PARAM POSX"},			//IDX_PARAM_DNLD_MENU
	{ "BACK",							"RECULER"},			//IDX_BACK
	{ "EDIT",							"EDITER"},			//IDX_EDIT
	{ "COPY",							"COPY"},				//IDX_COPY
	{ "SAVE",							"SAUVER"},			//IDX_SAVE
	{ "TEST",							"TEST"},			//IDX_TEST
	{ "PRODUCT",						"PRODUIT"},			//IDX_PRODUCT
	
	{ "FAILED TO"					, "F:FAILED TO"			}, // IDX_FAILED_TO
	{ "READ SWIPE"					, "F:READ SWIPE"			}, // IDX_READ_SWIPE
	{ "MERCHANT ID"					, "ID COMMERCANT"			}, // IDX_MERCHANT_ID  
	{ "TIMEZONE OFFSET"					, "F:TIMEZONE OFFSET"			}, // IDX_TIMEZONE_OFFSET  

	{ "END"								, "FIN"					}, // IDX_END

};

int CLanguage::m_usLanguage = 0;
HWND CLanguage::m_hWnd = 0;
BOOL  CLanguage::m_bPassToCustomer = FALSE;
int CLanguage::m_CustomerLanguage = 0;
BOOL  CLanguage::m_bLanguageSelected = FALSE;

CLanguage::CLanguage()
{
}
	
//-----------------------------------------------------------------------------
//!	\Set language
//!	\param	usLanguage - language id 0:English 1:french
//-----------------------------------------------------------------------------
void CLanguage::SetLanguage(int usLanguage)
{
	if ( usLanguage > 1)
		return;

	if( m_bLanguageSelected )
		return;
	m_bLanguageSelected = TRUE;	
	if ( m_usLanguage != usLanguage)
	{
		m_usLanguage = usLanguage;
		if( m_hWnd != 0)// && m_bPassToCustomer)
			::PostMessage(m_hWnd, WM_SOCK_RECV, 3, 0);
	}
}

//-----------------------------------------------------------------------------
//!	\Set default language
//-----------------------------------------------------------------------------
void CLanguage::SetLanguage()
{
	TCHAR buf1[10]={L""};
	DWORD language = ENGLISH;
	if(CDataFile::Read(L"LANGUAGE",buf1))
	{
		switch(buf1[0])
		{
		case '1':
			language = FRENCH;
			break;
		default:
			language = ENGLISH;
			break;
		}
	}
	if ( m_usLanguage != language  )
	{
		m_usLanguage = language;
		if( m_hWnd != 0)
			::PostMessage(m_hWnd, WM_SOCK_RECV, 3, 0);
	}

	m_usLanguage = language;
	m_CustomerLanguage = m_usLanguage;

	m_bLanguageSelected = FALSE;
}

//-----------------------------------------------------------------------------
//!	\Get language
//-----------------------------------------------------------------------------
int CLanguage::GetLanguage()
{
	if ( m_usLanguage > 1)
		m_usLanguage = 0;

	return m_usLanguage;
}

//-----------------------------------------------------------------------------
//!	\Get language text
//!	\param	usIndex - text id
//!	\param	pText - text buffer point
//-----------------------------------------------------------------------------
void CLanguage::GetTextFromIndex(int usIndex,char** pText)
{
	*pText = (char*)pbaDisplayText[usIndex][GetLanguage()];
}

//-----------------------------------------------------------------------------
//!	\Get language text
//!	\param	usIndex - text id
//-----------------------------------------------------------------------------
CString CLanguage::GetText(int usIndex)
{
	return CString((char*)pbaDisplayText[usIndex][GetLanguage()]);
}

//-----------------------------------------------------------------------------
//!	\Get language text
//!	\param	usIndex - text id
//-----------------------------------------------------------------------------
char* CLanguage::GetCharText(int usIndex)
{
	return (char*)pbaDisplayText[usIndex][GetLanguage()];
}
//-----------------------------------------------------------------------------
//!	\Set display window
//!	\param	hwnd - window handle
//-----------------------------------------------------------------------------
void CLanguage::SetDisplayWnd(HWND hwnd)
{
	m_hWnd = hwnd;
}

//-----------------------------------------------------------------------------
//!	\Set coustomer flag
//!	\param	flag - it is pass to coustomer or not
//-----------------------------------------------------------------------------
void CLanguage::SetCustomerFlag(BOOL flag)
{
	if ( m_bPassToCustomer != flag )
	{
		m_bPassToCustomer = flag;
		if( m_hWnd != 0 )
			::PostMessage(m_hWnd, WM_SOCK_RECV, 3, 0);
	}
}
