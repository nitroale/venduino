if (li < 1) {

    if (ti[0].power < temperature) {
      unsigned long oraInSecondi = now.hour() * 3600L;
      unsigned long minutiInSecondi = now.minute() * 60L;
      long oraInizio = ti[0].h1;
      long minutiInizio = ti[0].m1;
      long oraFine = ti[0].h2;
      long minutiFine = ti[0].m2;
      unsigned long secondiInizio = 0;
      unsigned long secondiFine = 0;
      unsigned long secondiAttuali = 0;
      secondiAttuali = oraInSecondi + minutiInSecondi;
      secondiInizio = (oraInizio * 3600 + minutiInizio * 60);
      secondiFine = (oraFine * 3600 + minutiFine * 60);

      if (secondiInizio < secondiFine) { // caso normale 8:00 18:00
        if (secondiInizio < secondiAttuali && secondiAttuali < secondiFine ) {
          out_s=ON;
          digitalWrite(ledVentola,HIGH);
        } else {
          out_s=OFF;
          digitalWrite(ledVentola,LOW);
        }
      } else {                  // caso 18:00 8:00
        if (secondiInizio < secondiAttuali || secondiAttuali < secondiFine) {
          out_s=ON;
          digitalWrite(ledVentola,HIGH);
        } else {
          out_s=OFF;
          digitalWrite(ledVentola,LOW);
        }
      }

  } else {
    out_s = OFF;
    digitalWrite(ledVentola,LOW);
  }
    // programma 2   temperatura per serpentina
  }