# V2 Expansion Port - Plan de Tests

Date de reference : 2026-05-08

## Objectif

Valider rapidement :
1. cablage expansion port
2. protocole minimal
3. reactivite des controles
4. robustesse sous charge

## Phase 0 - Bring-up des lignes

### Test 0.1 - OUT0-OUT2
- la ROM ecrit des patterns sur `$4016` bits `0-2`
- verifier au scope / logic analyzer que `OUT0-OUT2` togglent correctement

### Test 0.2 - Lecture D0-D4 via `$4017`
- le module force un pattern stable sur `D0-D4`
- la ROM lit `$4017`
- verifier la reconstruction cote NES

### Test 0.3 - Sniff D0-D7
- verifier que `D0-D7` sont observables
- verifier qu'ils ne sont jamais drives par la V2

## Phase 1 - Protocole minimal

But :
- implementer un seul registre logique

Premier candidat :
- `DUTY`

Validation :
- changement audible stable
- pas de glitch
- latence max acceptable en ticks

## Phase 2 - Controles riches

Ajouter progressivement :
- `ADSR` simplifiee
- `LFO rate`
- `LFO depth`

Validation :
- `120 Hz` sans stepping genant
- `240 Hz` en mode turbo si besoin

## Phase 3 - Stress tests

### Test 3.1 - MIDI dense
- flood notes + CC
- verifier absence de lock
- verifier absence de drift

### Test 3.2 - Protocole durci
- blocs invalides
- checksum faux
- pertes de sync volontaires

Validation :
- resync sans freeze

### Test 3.3 - Monitoring
- si `R/W` et `M2` sont repris, correler les phases CPU
- comparer ce que la ROM croit faire et ce que le bus montre

## Critere de succes

Le proto V2 est considere valide si :
- le cablage est stable
- un registre logique simple fonctionne
- les controles riches ne cassent pas le groove de V1
- la resynchronisation est robuste

## Ordre de progression recommande

1. `OUT0-OUT2`
2. `D0-D4`
3. `DUTY`
4. `ADSR`
5. `LFO`
6. stress tests

Voir aussi :
- [V2_EXPANSION_PORT_PIN_PLAN.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_PIN_PLAN.md)
- [V2_EXPANSION_PORT_PROTOCOL_SPEC.md](/C:/Users/mto1/Documents/NES_DEV/Project-V2-A/V2_EXPANSION_PORT_PROTOCOL_SPEC.md)
