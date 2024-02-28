steps:

- verificare che il vecchio firmware sia C3.5.2.5.STM
- caricare il firmware pasthrough all'interno dello zip en.inventek.updater.zip
- connettere STM32Programmer in modo serial (UART): dovrebbe risconoscere un processore M3 STM32 con 1MB di flash
- estrarre il nuovo wifi firmware ISM43362_M3G_L44_SPI_C3.5.2.7.STM.bin.rename dallo zip ISM43362_M3G_L44_SPI_C3.5.2.7.STM_.bin_.zip
- rinominare il .bin, togliendo l'estensione .rename
- programmare il bin ISM43362_M3G_L44_SPI_C3.5.2.7.STM.bin sulla board
- aspettare il completamento dell'upload sulla board
- fare reset della board (bottone nero)
- staccare e riattaccare il cavo USB della board al PC
- caricare il firmware NODES
- verificare che il nuovo firmware sia C3.5.2.7.STM
