
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
      var PACKAGE_NAME = '/Users/johnstiles/CandyCrisis/build-web/pkg_level_0.data';
      var REMOTE_PACKAGE_BASE = 'pkg_level_0.data';
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
Module['FS_createPath']("/CandyCrisisResources", "PICT_5000", true, true);

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
          Module['removeRunDependency']('datafile_/Users/johnstiles/CandyCrisis/build-web/pkg_level_0.data');
      }
      Module['addRunDependency']('datafile_/Users/johnstiles/CandyCrisis/build-web/pkg_level_0.data');

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
    loadPackage({"files": [{"filename": "/CandyCrisisResources/PICT_5000/Backdrop.png", "start": 0, "end": 198552}, {"filename": "/CandyCrisisResources/PICT_5000/BackdropPuppet-1P.png", "start": 198552, "end": 571689}, {"filename": "/CandyCrisisResources/PICT_5000/Board.png", "start": 571689, "end": 577289}, {"filename": "/CandyCrisisResources/PICT_5000/BoardGrays.png", "start": 577289, "end": 579144}, {"filename": "/CandyCrisisResources/PICT_5000/BoardNext.png", "start": 579144, "end": 583110}, {"filename": "/CandyCrisisResources/PICT_5000/BoardScore.png", "start": 583110, "end": 584827}, {"filename": "/CandyCrisisResources/PICT_5000/BoardSelect.png", "start": 584827, "end": 645216}, {"filename": "/CandyCrisisResources/PICT_5000/H-1A.png", "start": 645216, "end": 685332}, {"filename": "/CandyCrisisResources/PICT_5000/H-2A.png", "start": 685332, "end": 725679}, {"filename": "/CandyCrisisResources/PICT_5000/H-3A.png", "start": 725679, "end": 765976}, {"filename": "/CandyCrisisResources/PICT_5000/H-4A.png", "start": 765976, "end": 807607}, {"filename": "/CandyCrisisResources/PICT_5000/H-5A.png", "start": 807607, "end": 848109}, {"filename": "/CandyCrisisResources/PICT_5000/H-6A.png", "start": 848109, "end": 888225}, {"filename": "/CandyCrisisResources/PICT_5000/N-1A.png", "start": 888225, "end": 928686}, {"filename": "/CandyCrisisResources/PICT_5000/N-2A.png", "start": 928686, "end": 971773}, {"filename": "/CandyCrisisResources/PICT_5000/N-3A.png", "start": 971773, "end": 1015582}, {"filename": "/CandyCrisisResources/PICT_5000/N-4A.png", "start": 1015582, "end": 1057086}, {"filename": "/CandyCrisisResources/PICT_5000/N-5A.png", "start": 1057086, "end": 1095842}, {"filename": "/CandyCrisisResources/PICT_5000/S-1A.png", "start": 1095842, "end": 1137534}, {"filename": "/CandyCrisisResources/PICT_5000/S-2A.png", "start": 1137534, "end": 1178390}, {"filename": "/CandyCrisisResources/PICT_5000/S-3A.png", "start": 1178390, "end": 1220340}, {"filename": "/CandyCrisisResources/PICT_5000/S-4A.png", "start": 1220340, "end": 1262853}, {"filename": "/CandyCrisisResources/PICT_5000/S-5A.png", "start": 1262853, "end": 1301567}, {"filename": "/CandyCrisisResources/PICT_5000/S-6A.png", "start": 1301567, "end": 1349382}, {"filename": "/CandyCrisisResources/PICT_5000/S-6B.png", "start": 1349382, "end": 1399364}, {"filename": "/CandyCrisisResources/PICT_5000/Score.png", "start": 1399364, "end": 1407263}, {"filename": "/CandyCrisisResources/PICT_5000/ScoreInnerMask.png", "start": 1407263, "end": 1408361}, {"filename": "/CandyCrisisResources/PICT_5000/ScoreOuterMask.png", "start": 1408361, "end": 1409490}], "remote_package_size": 1409490});

  })();
