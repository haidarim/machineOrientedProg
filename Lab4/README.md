# Laboration 4: Undantagshantering

Under denna laboration utförs tre uppgifter som behandlas utförligt i läroboken där de beskrivs i form av deluppgifter som successivt måste klaras av innan själva laborationsuppgifterna kan lösas.

För att kunna genomföra hela laborationen måste du ha arbetat igenom hela kapitel 6 i läroboken. Du kan dock dela upp det så här:

Studera avsnitten t.o.m 6.2, speciellt exemplen och utför uppgifter t.o.m 6.3. Utför därefter laborationsuppgifter 4.1.
Avsnitten 6.3, 6.4 och 6.5 kan du sedan läsa kursivt. Dom är inte centrala för förståelsen av nästa uppgift.
Avsnitt 6.6 och 6.7 är grundläggande för laborationsuppgifter 4.4 - 4.6. Försäkra dig om att du förstår avsnitten. Utför därefter dessa laborationsuppgifter.

## Laborationsuppgift 4.1: Meddelandeskickning (Uppgift 6.3 i läroboken)

Uppgiften är att konstruera en avbrottsdriven fördröjningsrutin med SysTick-räknaren.

Kontrollera programmets funktion med CodeLite och SimServer (hemma) eller MD407/USBDM (vid laborationsplatsen.)
OBS! Vid debugging med USBDM behöver vi relokera vektortabellen samt starta klockorna för port D, E, och SYSCFG. Lägg till följande i början av app_init()

```c
    /* starta klockor port D och E */
    * ( (unsigned long *) 0x40023830) = 0x18;
    /* starta klockor för SYSCFG */
    * ((unsigned long *)0x40023844) |= 0x4000;
    /* Relokera vektortabellen */
    * ((unsigned long *)0xE000ED08) = 0x2001C000;
```

## Laborationsuppgift 4.2: Externavbrott, en avbrottsvektor (Uppgift 6.9 i läroboken)

Uppgiften är att konstruera en applikation med flera externa enheter som genererar avbrott och där samma avbrottsrutin används för att hantera samtliga avbrott.

Tips!
Eftersom vi bara använder en avbrottslina (EXTI3) i denna uppgift, kan vi inte använda EXTI Pending Register för att avgöra om IRQ0, IRQ1, eller IRQ2 har orsakat ett avbrott. Istället måste vi läsa pinnarna på E-porten (pin 0, 1, och 2) och avgöra vilka pinnar som är ettställda.

## Laborationsuppgift 4.3: Externavbrott, flera avbrottsvektorer (Uppgift 6.10 i läroboken)

I denna uppgift modifieras föregående applikation så att varje avbrott dirigeras till en egen avbrottsrutin.
