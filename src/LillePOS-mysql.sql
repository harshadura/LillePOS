USE LillePOS;

CREATE TABLE  `globals` (
  `name` text NOT NULL,
  `value` int(11),
  `strValue` text
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE  `categories` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE  `groups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `visible` tinyint(1) NOT NULL DEFAULT '1',
  `category` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `category` (`category`),
  CONSTRAINT `category` FOREIGN KEY (`category`) REFERENCES `categories` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE  `products` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` text NOT NULL,
  `price` double NOT NULL,
  `group` int(11) NOT NULL,
  `visible` tinyint(1) NOT NULL,
  `tax` int(11) NOT NULL DEFAULT '20',
  PRIMARY KEY (`id`),
  KEY `group` (`group`),
  CONSTRAINT `group` FOREIGN KEY (`group`) REFERENCES `groups` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE TABLE  `tickets` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `open` tinyint(1) NOT NULL DEFAULT '1' COMMENT 'is this ticket being worked on (not payed yet)',
  `timestamp` datetime NOT NULL,
  `invoiceNum` int(11) DEFAULT NULL COMMENT 'ticket number for accounting',
  `table` int(11) NOT NULL,
  `payedBy` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='they group orders per table';

CREATE TABLE  `orders` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `ticketId` INTEGER NOT NULL REFERENCES `tickets` (`id`),
  `product` INTEGER NOT NULL REFERENCES `products` (`id`),
  `count` int(11) NOT NULL DEFAULT '1',
  `price` double NOT NULL,
  `printed` int(11) NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

CREATE INDEX `orders_ticketId_index` ON `orders` (`ticketId`);

CREATE TABLE `orderExtras` (
  `orderId` INTEGER NOT NULL REFERENCES `orders` (`id`),
  `type` INTEGER,  /* "with" or "without" flag */
  `product` INTEGER NOT NULL REFERENCES `products` (`id`)
);

CREATE INDEX `orderExtras_orderId_index` ON `orderExtras` (`orderId`);

CREATE TABLE  `employees` (
  `id` INTEGER NOT NULL PRIMARY KEY AUTO_INCREMENT,
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
