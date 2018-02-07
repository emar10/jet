pipeline {
  agent any
  stages {
    stage('Build') {
      steps {
        sh '''make clean
make
mv ned ned-linux
make clean
make CC=/opt/osxcross/bin/o64-clang
mv ned ned-darwin'''
        archiveArtifacts 'ned-*'
      }
    }
  }
  environment {
    CC = 'gcc'
  }
}