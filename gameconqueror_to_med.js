// Node script to convert Older GameConqueror save file to Med save file
// The new file store the address as big integer,
// instead of hexadecimal

const fs = require('fs');

const convertToMedJson = (content) => {
  const parsed = JSON.parse(content);
  const { cheat_list: list } = parsed;

  const obj = {
    addresses: [],
    notes: ''
  };
  list.forEach(row => {
    const addressObj = {
      address: `0x${row[3]}`,
      description: row[2],
      lock: false,
      type: "int8",
      value: "0",
    };
    obj.addresses.push(addressObj);
  });
  return obj;
};

const main = argv => {
  if (argv.length < 4) {
    console.log(`Usage: ${__filename} input_file output_file`);
    process.exit(1);
  }
  const inputFile = argv[2];
  const outputFile = argv[3];

  const content = fs.readFileSync(inputFile, 'utf8');
  const converted = convertToMedJson(content);
  fs.writeFileSync(outputFile, JSON.stringify(converted));
  console.log('Done');
};

main(process.argv);
