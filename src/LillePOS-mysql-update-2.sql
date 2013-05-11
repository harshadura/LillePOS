BEGIN TRANSACTION;

-- change primary key of orders table to id column
CREATE TEMPORARY TABLE `orders_backup` (
  `ticketId` INTEGER NOT NULL,
  `product` INTEGER NOT NULL,
  `count` int(11) NOT NULL DEFAULT '1',
  `price` double NOT NULL,
  `printed` int(11) NOT NULL DEFAULT '0'
);

INSERT INTO orders_backup SELECT ticketId,product,count,price,printed FROM orders;

DROP TABLE orders;

CREATE TABLE  `orders` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `ticketId` INTEGER NOT NULL REFERENCES `tickets` (`id`),
  `product` INTEGER NOT NULL REFERENCES `products` (`id`),
  `count` int(11) NOT NULL DEFAULT '1',
  `price` double NOT NULL,
  `printed` int(11) NOT NULL DEFAULT '0'
);

INSERT INTO orders (ticketId,product,count,price,printed)
  SELECT ticketId,product,count,price,printed FROM orders_backup;

DROP TABLE orders_backup;

CREATE INDEX `orders_ticketId_index` ON `orders` (`ticketId`);

-- creat new table
CREATE TABLE `orderExtras` (
  `orderId` INTEGER NOT NULL REFERENCES `orders` (`id`),
  `type` INTEGER,  /* "with" or "without" flag */
  `product` INTEGER NOT NULL REFERENCES `products` (`id`)
);

CREATE INDEX `orderExtras_orderId_index` ON `orderExtras` (`orderId`);

-- add index to speedup reports
CREATE INDEX `workingTimes_employee_index` ON `workingTimes` (`employee`);

COMMIT;
