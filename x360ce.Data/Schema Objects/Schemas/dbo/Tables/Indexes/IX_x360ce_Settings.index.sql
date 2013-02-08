﻿CREATE UNIQUE NONCLUSTERED INDEX [IX_x360ce_Settings]
    ON [dbo].[x360ce_Settings]([SettingId] ASC, [FileName] ASC, [ProductName] ASC) WITH (ALLOW_PAGE_LOCKS = ON, ALLOW_ROW_LOCKS = ON, PAD_INDEX = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, IGNORE_DUP_KEY = OFF, STATISTICS_NORECOMPUTE = OFF, ONLINE = OFF, MAXDOP = 0)
    ON [PRIMARY];
