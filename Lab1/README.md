## Introduktion till laborationsdatorn MD407

Laborationsdatorn MD407 är baserad på mikrokontrollern STM32F407 som i sin tur är byggd kring 32-bitars-processorn ARM Cortex-M4 (168 MHz.) Utvecklingen av MD407 skedde på Chalmers Tekniska Högskola, institutionen för Data och Informationsteknik.

## Debugging med USBDM

Vi ska först testa att debugga ett program på MD407-kortet med hjälp av hårdvarugränsnittet USBDM.

## Laborationsuppgift 1.1 (uppgift 2.2 i läroboken)

I denna uppgift testar vi att i assembler-kod konfigurera GPIO-port D (utmatning) och E (inmatning.) Besvara frågan i kommentar i källkoden varför bit8-15 påverkas av LDRH/STRH men inte av LDRB/STRB.

## Laborationsuppgift 1.2 (uppgift 4.2 i läroboken)

Skriv en applikation som kontinuerligt läser av tangentbordet. Om någon tangent är nedtryckt, ska dess hexadecimala tangentkod skrivas till 7-segmentsdisplayen. Om ingen tangent är nedtryckt ska displayen släckas.

## Laborationsuppgift 1.3

Modifiera tangentbordsrutinen från uppgift 1.2 så att varje nedtryckning detekteras exakt en gång.

## Laborationsuppgift 1.4

Konstruera en tangentbordsrutin som returnerar ett statusord (16 bitar) där varje tangent har en bitposition. Bitvärdet 1 indikerar en nedtryckt tangent medan 0 indikerar en uppsläppt tangent.
