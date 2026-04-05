
  var Module = typeof Module != 'undefined' ? Module : {};

  if (!Module['expectedDataFileDownloads']) Module['expectedDataFileDownloads'] = 0;
  Module['expectedDataFileDownloads']++;
  (() => {
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;
    var isNode = globalThis.process && globalThis.process.versions && globalThis.process.versions.node && globalThis.process.type != 'renderer';
    async function loadPackage(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'pkg_world3.data';
      var REMOTE_PACKAGE_BASE = 'pkg_world3.data';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
      var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      async function fetchRemotePackage(packageName, packageSize) {
        if (isNode) {
          var contents = require('fs').readFileSync(packageName);
          return new Uint8Array(contents).buffer;
        }
        if (!Module['dataFileDownloads']) Module['dataFileDownloads'] = {};
        try {
          var response = await fetch(packageName);
        } catch (e) {
          throw new Error(`Network Error: ${packageName}`, {e});
        }
        if (!response.ok) {
          throw new Error(`${response.status}: ${response.url}`);
        }

        const chunks = [];
        const headers = response.headers;
        const total = Number(headers.get('Content-Length') || packageSize);
        let loaded = 0;

        Module['setStatus'] && Module['setStatus']('Downloading data...');
        const reader = response.body.getReader();

        while (1) {
          var {done, value} = await reader.read();
          if (done) break;
          chunks.push(value);
          loaded += value.length;
          Module['dataFileDownloads'][packageName] = {loaded, total};

          let totalLoaded = 0;
          let totalSize = 0;

          for (const download of Object.values(Module['dataFileDownloads'])) {
            totalLoaded += download.loaded;
            totalSize += download.total;
          }

          Module['setStatus'] && Module['setStatus'](`Downloading data... (${totalLoaded}/${totalSize})`);
        }

        const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
        let offset = 0;
        for (const chunk of chunks) {
          packageData.set(chunk, offset);
          offset += chunk.length;
        }
        return packageData.buffer;
      }

      var fetchPromise;
      var fetched = Module['getPreloadedPackage'] && Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);

      if (!fetched) {
        // Note that we don't use await here because we want to execute the
        // the rest of this function immediately.
        fetchPromise = fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);
      }

    async function runWithFS(Module) {

      function assert(check, msg) {
        if (!check) throw new Error(msg);
      }
Module['FS_createPath']("/", "CandyCrisisResources", true, true);

    for (var file of metadata['files']) {
      var name = file['filename']
      Module['addRunDependency'](`fp ${name}`);
    }

      async function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData ' + arrayBuffer.constructor.name);
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          for (var file of metadata['files']) {
            var name = file['filename'];
            var data = byteArray.subarray(file['start'], file['end']);
            // canOwn this data in the filesystem, it is a slice into the heap that will never change
        Module['FS_createDataFile'](name, null, data, true, true, true);
        Module['removeRunDependency'](`fp ${name}`);
          }
          Module['removeRunDependency']('datafile_pkg_world3.data');
      }
      Module['addRunDependency']('datafile_pkg_world3.data');

      if (!Module['preloadResults']) Module['preloadResults'] = {};

      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (!fetched) {
        fetched = await fetchPromise;
      }
      processPackageData(fetched);

    }
    if (Module['calledRun']) {
      runWithFS(Module);
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module['preRun'].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_617.png", "start": 0, "end": 568070}, {"filename": "/CandyCrisisResources/PICT_618.png", "start": 568070, "end": 656951}, {"filename": "/CandyCrisisResources/PICT_619.png", "start": 656951, "end": 849024}, {"filename": "/CandyCrisisResources/PICT_620.png", "start": 849024, "end": 936644}, {"filename": "/CandyCrisisResources/PICT_621.png", "start": 936644, "end": 1001159}, {"filename": "/CandyCrisisResources/PICT_630.jpg", "start": 1001159, "end": 1031819}, {"filename": "/CandyCrisisResources/PICT_631.png", "start": 1031819, "end": 1079707}, {"filename": "/CandyCrisisResources/PICT_632.png", "start": 1079707, "end": 1119692}, {"filename": "/CandyCrisisResources/PICT_633.png", "start": 1119692, "end": 1480000}, {"filename": "/CandyCrisisResources/PICT_634.png", "start": 1480000, "end": 1830666}, {"filename": "/CandyCrisisResources/PICT_635.png", "start": 1830666, "end": 2264718}, {"filename": "/CandyCrisisResources/PICT_636.png", "start": 2264718, "end": 3655943}, {"filename": "/CandyCrisisResources/PICT_637.png", "start": 3655943, "end": 4228937}, {"filename": "/CandyCrisisResources/PICT_638.png", "start": 4228937, "end": 4231204}, {"filename": "/CandyCrisisResources/PICT_639.png", "start": 4231204, "end": 4233985}, {"filename": "/CandyCrisisResources/PICT_640.png", "start": 4233985, "end": 4239666}, {"filename": "/CandyCrisisResources/PICT_641.png", "start": 4239666, "end": 4244960}, {"filename": "/CandyCrisisResources/PICT_642.png", "start": 4244960, "end": 4347792}, {"filename": "/CandyCrisisResources/PICT_643.png", "start": 4347792, "end": 4353496}, {"filename": "/CandyCrisisResources/PICT_644.png", "start": 4353496, "end": 4366928}, {"filename": "/CandyCrisisResources/PICT_645.png", "start": 4366928, "end": 5468911}, {"filename": "/CandyCrisisResources/PICT_646.png", "start": 5468911, "end": 5649380}, {"filename": "/CandyCrisisResources/PICT_647.png", "start": 5649380, "end": 7011127}, {"filename": "/CandyCrisisResources/PICT_648.png", "start": 7011127, "end": 7014819}, {"filename": "/CandyCrisisResources/PICT_649.png", "start": 7014819, "end": 7022705}], "remote_package_size": 7022705});

  })();
