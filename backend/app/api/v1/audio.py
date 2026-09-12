from uuid import UUID

from fastapi import APIRouter, File, Request, UploadFile
from sqlalchemy.ext.asyncio import AsyncSession
from starlette.datastructures import UploadFile as StarletteUploadFile

from app.api.deps import SessionDep, TaskRunnerDep
from app.core.exceptions import SonoraError
from app.schemas.audio import (
    AnalysisOut,
    AnalyzeByIdRequest,
    AudioAssetOut,
    AudioDetailResponse,
)
from app.schemas.common import EnqueuedAnalysisResponse
from app.services import analysis as analysis_service
from app.services.task_runner import TaskRunner

router = APIRouter(prefix="/audio", tags=["audio"])


@router.post("/upload", response_model=AudioAssetOut, status_code=201)
async def upload_audio(
    file: UploadFile = File(...),
    session: AsyncSession = SessionDep,
) -> AudioAssetOut:
    data = await file.read()
    filename = file.filename or "upload.wav"
    asset = await analysis_service.create_asset_from_bytes(
        session, filename=filename, data=data
    )
    return AudioAssetOut.model_validate(asset)


@router.post("/analyze", response_model=EnqueuedAnalysisResponse, status_code=202)
async def analyze_audio(
    request: Request,
    session: AsyncSession = SessionDep,
    runner: TaskRunner = TaskRunnerDep,
) -> EnqueuedAnalysisResponse:
    content_type = request.headers.get("content-type", "")
    if "application/json" in content_type:
        body = AnalyzeByIdRequest.model_validate(await request.json())
        analysis = await analysis_service.enqueue_analysis(
            session, runner, audio_id=body.audio_id
        )
        return EnqueuedAnalysisResponse(
            analysis_id=analysis.id,
            audio_id=analysis.audio_id,
            status=analysis.status,
        )

    if "multipart/form-data" in content_type:
        form = await request.form()
        upload = form.get("file")
        if not isinstance(upload, StarletteUploadFile):
            raise SonoraError("multipart field 'file' is required")
        data = await upload.read()
        filename = upload.filename or "upload.wav"
        _asset, analysis = await analysis_service.upload_and_analyze(
            session, runner, filename=filename, data=data
        )
        return EnqueuedAnalysisResponse(
            analysis_id=analysis.id,
            audio_id=analysis.audio_id,
            status=analysis.status,
        )

    raise SonoraError("Provide a multipart file or JSON {audio_id}")


@router.get("/{audio_id}", response_model=AudioDetailResponse)
async def get_audio(
    audio_id: UUID,
    session: AsyncSession = SessionDep,
) -> AudioDetailResponse:
    asset, analysis = await analysis_service.get_audio_detail(session, audio_id)
    return AudioDetailResponse(
        asset=AudioAssetOut.model_validate(asset),
        analysis=AnalysisOut.model_validate(analysis) if analysis else None,
    )
