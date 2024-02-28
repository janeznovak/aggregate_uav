# Steps for hospital demo

## Notes
paziente 1: in alto
paziente 2: in basso

## Steps

i robot 1 e 2 sono dotati di webcam: per gestire i problemi relativi al cuore;  
i robot 3 e 4 sono dotati di microfono: per gestire i problemi relativi alla caduta accidentale dei pazienti.

___

paziente 2 problema di saturazione:  
a seconda di quello più vicino a lui, si muove il robot tra il 3 e il 4

___

paziente 1 caduta accidentale:  
si muoverà un robot tra 3 e 4 verso il paziente 1, in modo prioritario.

___

nel frattempo, creo un evento di tipo manuale per il paziente 2.  
il sistema resta in attesa.  
si muoverà il robot rimasto nella stanza in alto.

____

simulare la rottura del robot in alto:  
paziente 2 problema di temperatura.  
arriverà in soccorso un robot dall'altra stanza.
