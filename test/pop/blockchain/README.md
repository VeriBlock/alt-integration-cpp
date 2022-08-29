#### vbk_testnet30000.txt

Contains:
```
block hash
cumulative_difficulty
header
```

Selected with:
```sql
select s.hash, cumulative_difficulty, header from block_scores as s, blocks as b
where s.hash = b.hash and number < 30000;
```
