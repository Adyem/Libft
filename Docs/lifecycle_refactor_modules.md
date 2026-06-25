# Lifecycle Refactor Module Checklist

This checklist tracks the migration away from lifecycle copy/move constructors and
return-by-value lifecycle factories. Module order follows the root `Makefile`.

| Module | Lifecycle files | Status |
| --- | ---: | --- |
| Basic | 0 | Clear |
| Advanced | 0 | Clear |
| Compatebility | 0 | Clear |
| Errno | 0 | Clear |
| CMA | 2 | Pending |
| SCMA | 1 | Pending |
| GetNextLine | 2 | Complete: `gnl_stream` copy/move constructors deleted; no lifecycle value-return factory present |
| DUMB | 6 | Complete - lifecycle copy/move constructors deleted and module build passes |
| Math | 17 | Partial: `ft_cubic_spline` factory now returns a heap pointer and copy/move constructors are deleted |
| Geometry | 6 | Complete: `aabb`, `circle`, and `sphere` copy/move constructors deleted; no lifecycle value-return factory present |
| System_utils | 0 | Clear |
| Printf | 0 | Clear |
| ReadLine | 0 | Clear |
| PThread | 7 | Pending: low-level compatibility exemptions apply |
| CPP_class | 23 | Pending: several value-semantic exceptions apply |
| Template | 23 | Pending: many documented compatibility exceptions apply |
| Buffer | 2 | Complete: `ft_byte_buffer` copy/move constructors deleted; no lifecycle value-return factory present |
| CLI | 0 | Clear |
| Command | 0 | Clear |
| Config | 0 | Clear |
| CrossProcess | 0 | Clear |
| Compression | 2 | Complete: lifecycle copy/move constructors already deleted; no lifecycle value-return factory present |
| Encryption | 2 | Complete: `encryption_aead_context` copy/move constructors deleted; explicit `initialize(...)` paths retained |
| Encoding | 0 | Clear |
| RNG | 4 | Complete: `rng_stream`, `ft_deck`, `ft_loot_entry`, and `ft_loot_table` copy/move constructors deleted |
| JSon | 2 | Complete: `json_document` copy/move constructors deleted; no lifecycle value-return factory present |
| YAML | 2 | Complete: `yaml_value` copy/move constructors deleted; heap parser APIs return `ft_nullptr` on error |
| File | 2 | Complete: `ft_file_watch` copy/move constructors deleted; no lifecycle value-return factory present |
| HTML | 2 | Complete: `html_document` copy/move constructors deleted; no lifecycle value-return factory present |
| Time | 4 | Complete: `time_timer` and `time_fps` copy/move constructors deleted; non-class lifecycle structs unchanged |
| Filesystem | 0 | Clear |
| XML | 2 | Complete: `xml_document` copy/move constructors deleted; `xml_node` was already non-copyable/non-movable |
| Storage | 5 | Complete: `kv_store_entry` and `kv_store` copy/move constructors deleted; no lifecycle value-return factory present |
| Networking | 19 | Complete - copy/move constructors deleted and module build passes |
| API | 9 | Complete - lifecycle copy/move constructors deleted and module build passes |
| Observability | 0 | Clear |
| Logger | 3 | Complete - copy/move constructors deleted, context guard factory returns a heap pointer, and module build passes |
| Parser | 7 | Complete - copy/move constructors deleted, DOM attribute lookup returns a heap pointer, and Parser/JSon builds pass |
| Game | 81 | Pending |

## Working Order

1. GetNextLine
2. Buffer
3. Compression
4. Encryption
5. JSon
6. YAML
7. File
8. HTML
9. XML
10. RNG
11. Geometry
12. Time
13. Storage
14. Logger
15. Parser
16. Networking
17. API
18. DUMB
19. Math
20. CMA
21. SCMA
22. PThread
23. CPP_class
24. Template
25. Game
