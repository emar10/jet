pipeline {
  agent any
  stages {
    stage('Build') {
      steps {
        sh '''make clean
make
mv jet jet-linux
make clean
make CC=/opt/osxcross/bin/o64-clang
mv jet jet-darwin'''
        archiveArtifacts 'jet-*'
      }
    }
  }
  environment {
    CC = 'gcc'
  }
}
