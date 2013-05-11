CREATE TABLE  `categories` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `name` text NOT NULL
);

CREATE TABLE  `globals` (
  `name` text NOT NULL,
  `value` int(11),
  `strValue` text
);


CREATE TABLE  `groups` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `name` text NOT NULL,
  `visible` tinyint(1) NOT NULL DEFAULT '1',
  `category` int(11) NOT NULL CONSTRAINT `category` REFERENCES `categories` (`id`)
);


CREATE TABLE  `orders` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `ticketId` INTEGER NOT NULL REFERENCES `tickets` (`id`),
  `product` INTEGER NOT NULL REFERENCES `products` (`id`),
  `count` int(11) NOT NULL DEFAULT '1',
  `price` double NOT NULL,
  `printed` int(11) NOT NULL DEFAULT '0'
);

CREATE INDEX `orders_ticketId_index` ON `orders` (`ticketId`);

CREATE TABLE `orderExtras` (
  `orderId` INTEGER NOT NULL REFERENCES `orders` (`id`),
  `type` INTEGER,  /* "with" or "without" flag */
  `product` INTEGER NOT NULL REFERENCES `products` (`id`)
);

CREATE INDEX `orderExtras_orderId_index` ON `orderExtras` (`orderId`);

CREATE TABLE  `products` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `name` text NOT NULL,
  `price` double NOT NULL,
  `group` int(11) NOT NULL,
  `visible` tinyint(1) NOT NULL,
  `tax` int(11) NOT NULL DEFAULT '20',
  CONSTRAINT `group` FOREIGN KEY (`group`) REFERENCES `groups` (`id`)
);

CREATE TABLE  `tickets` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `open` tinyint(1) NOT NULL DEFAULT '1',
  `timestamp` datetime NOT NULL,
  `invoiceNum` int(11) DEFAULT NULL,
  `table` int(11) NOT NULL,
  `payedBy` int(11) NOT NULL DEFAULT '0'
);

CREATE TABLE  `employees` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `name` text NOT NULL,
  `start` datetime
);

CREATE TABLE  `workingTimes` (
  `employee` INTEGER NOT NULL,
  `start` datetime,
  `end` datetime,
  CONSTRAINT `employee` FOREIGN KEY (`employee`) REFERENCES `employees` (`id`)
);

CREATE INDEX `workingTimes_employee_index` ON `workingTimes` (`employee`);
